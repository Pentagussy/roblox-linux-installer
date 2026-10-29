#include "http.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>

namespace http {

namespace {

size_t writeStringCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

struct DownloadContext {
    std::ofstream file;
    ProgressCallback callback;
};

size_t writeFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    DownloadContext* ctx = static_cast<DownloadContext*>(userp);
    ctx->file.write(static_cast<char*>(contents), total);
    return total;
}

int xferInfoCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    DownloadContext* ctx = static_cast<DownloadContext*>(clientp);
    if (ctx && ctx->callback && dltotal > 0) {
        ctx->callback(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
    }
    return 0;
}

} // anonymous namespace

std::string get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "OpenInstaller/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code < 200 || http_code >= 300) {
        return "";
    }
    return response;
}

bool downloadFile(const std::string& url, const fs::path& destPath, ProgressCallback progressCb) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    fs::create_directories(destPath.parent_path());
    fs::path tempPath = destPath;
    tempPath += ".tmp";

    DownloadContext ctx;
    ctx.file.open(tempPath, std::ios::binary);
    ctx.callback = progressCb;

    if (!ctx.file.is_open()) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "OpenInstaller/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferInfoCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, progressCb ? 0L : 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    ctx.file.close();
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && http_code >= 200 && http_code < 300) {
        std::error_code ec;
        fs::rename(tempPath, destPath, ec);
        return !ec;
    }

    std::error_code ec;
    fs::remove(tempPath, ec);
    return false;
}

} // namespace http
