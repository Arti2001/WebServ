#include "MimeTypes.hpp"

static const std::unordered_map<std::string, std::string> typesOfMime = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".ico", "image/x-icon"},
    {".svg", "image/svg+xml"},
    {".txt", "text/plain"}
};

std::string MimeTypes::detectMimeType(const std::string& file_path) {
    auto pos = file_path.rfind('.');
    if (pos != std::string::npos) {
        std::string ext = file_path.substr(pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        auto it = typesOfMime.find(ext);
        if (it != typesOfMime.end())
            return it->second;
    }
    return "application/octet-stream";
}
