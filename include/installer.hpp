#pragma once

#include "config.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Installer {
public:
    Installer(fs::path installDir = "", std::string channel = "LIVE");

    bool run(const std::vector<std::string>& clientArgs = {}, bool force = false);

    std::string fetchRemoteVersionGuid() const;
    std::string getLocalVersionGuid() const;
    void saveLocalVersionGuid(const std::string& guid) const;

    bool installVersion(const std::string& versionGuid, const fs::path& targetDir);
    fs::path findClientExecutable(const fs::path& versionDir) const;
    bool launchClient(const fs::path& clientExe, const std::vector<std::string>& args) const;

private:
    fs::path m_baseDir;
    fs::path m_versionsDir;
    fs::path m_downloadsDir;
    std::string m_channel;
};
