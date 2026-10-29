#pragma once

#include <string>
#include <vector>
#include <utility>
#include <filesystem>

namespace fs = std::filesystem;

namespace config {

inline const std::string CDN_BASE_URL = "https://setup.rbxcdn.com";
inline const std::string CLIENT_SETTINGS_CDN = "https://clientsettingscdn.roblox.com";

// 1:1 Package extraction map reversed from installer.exe binary
inline const std::vector<std::pair<std::string, std::string>> PACKAGE_MAP = {
    {"RobloxApp.zip", ""},
    {"content-avatar.zip", "content/avatar/"},
    {"content-configs.zip", "content/configs/"},
    {"content-fonts.zip", "content/fonts/"},
    {"content-sky.zip", "content/sky/"},
    {"content-sounds.zip", "content/sounds/"},
    {"content-textures2.zip", "content/textures/"},
    {"content-models.zip", "content/models/"},
    {"content-textures3.zip", "PlatformContent/pc/textures/"},
    {"content-terrain.zip", "PlatformContent/pc/terrain/"},
    {"content-platform-fonts.zip", "PlatformContent/pc/fonts/"},
    {"content-platform-dictionaries.zip", "PlatformContent/pc/shared_compression_dictionaries/"},
    {"extracontent-luapackages.zip", "ExtraContent/LuaPackages/"},
    {"extracontent-translations.zip", "ExtraContent/translations/"},
    {"extracontent-models.zip", "ExtraContent/models/"},
    {"extracontent-textures.zip", "ExtraContent/textures/"},
    {"extracontent-places.zip", "ExtraContent/places/"},
    {"shaders.zip", "shaders/"},
    {"ssl.zip", "ssl/"},
    {"redist.zip", ""},
    {"WebView2.zip", ""},
    {"WebView2RuntimeInstaller.zip", "WebView2RuntimeInstaller/"}
};

fs::path getDefaultInstallDir();

} // namespace config
