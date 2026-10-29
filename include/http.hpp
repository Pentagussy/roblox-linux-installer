#pragma once

#include <string>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

namespace http {

using ProgressCallback = std::function<void(size_t downloaded, size_t total)>;

std::string get(const std::string& url);

bool downloadFile(
    const std::string& url,
    const fs::path& destPath,
    ProgressCallback progressCb = nullptr
);

} // namespace http
