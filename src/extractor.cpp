#include "extractor.hpp"
#include <zip.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>

namespace extractor {

bool extractZip(const fs::path& zipPath, const fs::path& destDir) {
    int err = 0;
    zip_t* za = zip_open(zipPath.string().c_str(), 0, &err);
    if (!za) {
        return false;
    }

    fs::create_directories(destDir);

    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    std::vector<char> buffer(64 * 1024);

    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(za, i, 0);
        if (!name) continue;

        fs::path entryRelPath(name);
        fs::path targetPath = destDir / entryRelPath;

        // Skip directory entries ending with slash
        if (name[strlen(name) - 1] == '/' || name[strlen(name) - 1] == '\\') {
            fs::create_directories(targetPath);
            continue;
        }

        fs::create_directories(targetPath.parent_path());

        zip_file_t* zf = zip_fopen_index(za, i, 0);
        if (!zf) continue;

        std::ofstream out(targetPath, std::ios::binary);
        if (out.is_open()) {
            zip_int64_t bytes_read = 0;
            while ((bytes_read = zip_fread(zf, buffer.data(), buffer.size())) > 0) {
                out.write(buffer.data(), bytes_read);
            }
            out.close();
        }
        zip_fclose(zf);
    }

    zip_close(za);
    return true;
}

} // namespace extractor
