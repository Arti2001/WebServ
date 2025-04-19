// MIME (Multipurpose Internet Mail Extensions)

#ifndef MIME_TYPES_HPP
#define MIME_TYPES_HPP

#include <string>
#include <unordered_map>
#include <algorithm>

class MimeTypes {
    public:
        static std::string detectMimeType(const std::string& file_path);
};

#endif