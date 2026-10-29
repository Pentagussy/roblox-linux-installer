#include "installer.hpp"
#include "http.hpp"
#include "extractor.hpp"
#include <iostream>
#include <fstream>
#include <regex>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace config {

fs::path getDefaultInstallDir() {
#ifdef _WIN32
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData) {
        return fs::path(localAppData) / "Roblox";
    }
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        return fs::path(userProfile) / "AppData" / "Local" / "Roblox";
    }
    return fs::current_path() / "Roblox";
#else
    const char* xdgData = std::getenv("XDG_DATA_HOME");
    if (xdgData) {
        return fs::path(xdgData) / "roblox";
    }
    const char* home = std::getenv("HOME");
    if (home) {
        return fs::path(home) / ".local" / "share" / "roblox";
    }
    return fs::current_path() / "roblox";
#endif
}

} // namespace config

Installer::Installer(fs::path installDir, std::string channel)
    : m_channel(std::move(channel)) {
    if (installDir.empty()) {
        m_baseDir = config::getDefaultInstallDir();
    } else {
        m_baseDir = std::move(installDir);
    }
    m_versionsDir = m_baseDir / "Versions";
    m_downloadsDir = m_baseDir / "Downloads";
}

std::string Installer::fetchRemoteVersionGuid() const {
    // 1. Try clientsettings CDN API
    std::string apiUrl = config::CLIENT_SETTINGS_CDN + "/v2/client-version/WindowsPlayer";
    if (!m_channel.empty() && m_channel != "LIVE") {
        apiUrl += "/channel/" + m_channel;
    }

    std::string jsonResp = http::get(apiUrl);
    if (!jsonResp.empty()) {
        // Regex search for "clientVersionUpload":"(version-[a-f0-9]+)"
        std::regex guidRegex("\"clientVersionUpload\"\\s*:\\s*\"([^\"]+)\"");
        std::smatch match;
        if (std::regex_search(jsonResp, match, guidRegex) && match.size() > 1) {
            return match[1].str();
        }
    }

    // 2. Fallback to setup.rbxcdn.com/version
    std::string fallbackUrl = config::CDN_BASE_URL + "/version";
    std::string verText = http::get(fallbackUrl);
    // Trim whitespace
    while (!verText.empty() && (verText.back() == '\r' || verText.back() == '\n' || verText.back() == ' ')) {
        verText.pop_back();
    }
    return verText;
}

std::string Installer::getLocalVersionGuid() const {
    fs::path verFile = m_baseDir / "version.txt";
    if (fs::exists(verFile)) {
        std::ifstream in(verFile);
        std::string line;
        if (std::getline(in, line)) {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
                line.pop_back();
            }
            return line;
        }
    }
    return "";
}

void Installer::saveLocalVersionGuid(const std::string& guid) const {
    fs::create_directories(m_baseDir);
    fs::path verFile = m_baseDir / "version.txt";
    std::ofstream out(verFile);
    if (out.is_open()) {
        out << guid << "\n";
    }
}

fs::path Installer::findClientExecutable(const fs::path& versionDir) const {
    const std::vector<std::string> candidates = {
        "RobloxPlayerBeta.exe",
        "RobloxPlayerLauncher.exe",
        "RobloxPlayer.exe"
    };

    for (const auto& name : candidates) {
        fs::path p = versionDir / name;
        if (fs::exists(p)) return p;
    }

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(versionDir, ec)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.rfind("Roblox", 0) == 0 && entry.path().extension() == ".exe") {
                return entry.path();
            }
        }
    }

    return "";
}

bool Installer::installVersion(const std::string& versionGuid, const fs::path& targetDir) {
    fs::create_directories(m_downloadsDir);
    fs::create_directories(targetDir);

    size_t total = config::PACKAGE_MAP.size();
    size_t current = 0;

    for (const auto& [pkgName, relPath] : config::PACKAGE_MAP) {
        current++;
        fs::path zipFile = m_downloadsDir / (versionGuid + "-" + pkgName);
        std::string downloadUrl = config::CDN_BASE_URL + "/" + versionGuid + "-" + pkgName;

        std::cout << "[" << current << "/" << total << "] Downloading " << pkgName << "...\n";

        bool ok = http::downloadFile(downloadUrl, zipFile, [](size_t dl, size_t total) {
            if (total > 0) {
                int pct = static_cast<int>((static_cast<double>(dl) / total) * 100.0);
                std::cout << "\r    -> " << pct << "% (" << dl / 1024 << " KB / " << total / 1024 << " KB)" << std::flush;
            }
        });

        std::cout << "\n";

        if (!ok) {
            std::cout << "    [-] Package unavailable or optional, skipping: " << pkgName << "\n";
            continue;
        }

        std::cout << "    [+] Extracting " << pkgName << " to " << (relPath.empty() ? "." : relPath) << "\n";
        fs::path extractDir = targetDir / relPath;
        if (!extractor::extractZip(zipFile, extractDir)) {
            std::cerr << "    [!] Failed to extract " << pkgName << "\n";
        }
    }

    std::error_code ec;
    fs::remove_all(m_downloadsDir, ec);
    return true;
}

bool Installer::launchClient(const fs::path& clientExe, const std::vector<std::string>& args) const {
#ifdef _WIN32
    std::string cmd = "\"" + clientExe.string() + "\"";
    for (const auto& a : args) {
        cmd += " \"" + a + "\"";
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back('\0');

    BOOL res = CreateProcessA(
        NULL,
        cmdBuffer.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        clientExe.parent_path().string().c_str(),
        &si,
        &pi
    );

    if (res) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
#else
    // Linux: launch through wine if binary is .exe
    std::string cmd = "wine \"" + clientExe.string() + "\"";
    for (const auto& a : args) {
        cmd += " \"" + a + "\"";
    }
    cmd += " >/dev/null 2>&1 &";
    int res = std::system(cmd.c_str());
    return (res == 0);
#endif
}

bool Installer::run(const std::vector<std::string>& clientArgs, bool force) {
    std::cout << "[*] Querying latest version GUID from CDN...\n";
    std::string remoteGuid = fetchRemoteVersionGuid();
    if (remoteGuid.empty()) {
        std::cerr << "[!] Could not resolve latest version GUID from CDN.\n";
        return false;
    }

    std::cout << "[+] Latest version: " << remoteGuid << "\n";
    std::string localGuid = getLocalVersionGuid();
    fs::path versionDir = m_versionsDir / remoteGuid;

    bool needsInstall = force || (localGuid != remoteGuid) || !fs::exists(versionDir) || findClientExecutable(versionDir).empty();

    if (needsInstall) {
        std::cout << "[*] Performing installation into " << versionDir << "...\n";
        if (!installVersion(remoteGuid, versionDir)) {
            std::cerr << "[!] Installation failed.\n";
            return false;
        }
        saveLocalVersionGuid(remoteGuid);
        std::cout << "[+] Installation complete!\n";
    } else {
        std::cout << "[+] Client is already up to date.\n";
    }

    fs::path exePath = findClientExecutable(versionDir);
    if (exePath.empty()) {
        std::cerr << "[!] Client executable not found in " << versionDir << "\n";
        return false;
    }

    std::cout << "[*] Spawning client: " << exePath.filename() << "\n";
    return launchClient(exePath, clientArgs);
}
