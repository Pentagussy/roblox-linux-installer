#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace extractor {

bool extractZip(const fs::path& zipPath, const fs::path& destDir);

} // namespace extractor
