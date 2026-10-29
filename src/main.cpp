#include "installer.hpp"
#include <iostream>
#include <string>
#include <vector>

void printHelp(const char* progName) {
    std::cout << "OpenInstaller - Open-Source Native Client Bootstrapper\n\n"
              << "Usage: " << progName << " [OPTIONS] [-- [CLIENT_ARGS...]]\n\n"
              << "Options:\n"
              << "  -h, --help            Show this help text\n"
              << "  -c, --channel <name>  Deployment channel (default: LIVE)\n"
              << "  -d, --dir <path>      Custom installation directory\n"
              << "  -f, --force           Force re-download and re-install\n\n"
              << "Examples:\n"
              << "  " << progName << "\n"
              << "  " << progName << " --channel zcanary\n"
              << "  " << progName << " --dir ./my_roblox\n"
              << "  " << progName << " -- --app -t <token>\n";
}

int main(int argc, char* argv[]) {
    std::string channel = "LIVE";
    fs::path customDir;
    bool force = false;
    std::vector<std::string> clientArgs;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if ((arg == "-c" || arg == "--channel") && i + 1 < argc) {
            channel = argv[++i];
        } else if ((arg == "-d" || arg == "--dir") && i + 1 < argc) {
            customDir = argv[++i];
        } else if (arg == "-f" || arg == "--force") {
            force = true;
        } else if (arg == "--") {
            // All subsequent arguments belong to the client
            for (int j = i + 1; j < argc; ++j) {
                clientArgs.push_back(argv[j]);
            }
            break;
        } else {
            // Forward unknown flags to client
            clientArgs.push_back(arg);
        }
    }

    Installer installer(customDir, channel);
    if (!installer.run(clientArgs, force)) {
        return 1;
    }
    return 0;
}
