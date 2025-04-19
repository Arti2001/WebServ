#include "StaticHandler.hpp"

StaticHandler::StaticHandler() {}

StaticHandler::~StaticHandler() {}

Response StaticHandler::serve(const HTTPRequest& req, const Location& loc) {
    Response resp;
    // Method check
    bool allowed = false;
    for (auto& m : loc._allowedMethods) {
        if (m == req.getMethod()) {
            allowed = true;
            break;
        }
    }
    if (!allowed) {
        resp.setStatusCode(405);
        resp.setReasonPhrase("Method Not Allowed");
        std::ostringstream allow;
        for (size_t i = 0; i < loc._allowedMethods.size(); i++) {
            if (i) allow << ", ";
            allow << loc._allowedMethods[i];
        }
        resp.addHeader("Allow", allow.str());
        return loadErrorPage(loc, 405);
    }

    // Path check
    std::string uri = req.getUri();
    std::string path = uri.substr(loc._path.length());
    if (path.empty() || path.back() == '/')
        path += loc._index;
    std::string fullPath = loc._root + path;

    // Stat
    struct stat sb;
    if (stat(fullPath.c_str(), &sb) < 0) {
        return loadErrorPage(loc, 404);
    }

    // Directory vs File
    if (S_ISDIR(sb.st_mode)) {
        if (!loc._auto_index) {
            return loadErrorPage(loc, 403);
    }
    auto listing = generateDirectoryListing(fullPath, uri);
    resp.setBody(std::move(listing));
    resp.setStatusCode(200);
    resp.setReasonPhrase("OK");
    resp.addHeader("Content-Type", "text/html");
    } else {
        int fd = open(fullPath.c_str(), O_RDONLY);
        if (fd < 0)
            return loadErrorPage(loc, 404);
        std::vector<char> buffer(sb.st_size);
        ssize_t bytesRead = read(fd, buffer.data(), sb.st_size);
        close(fd);
        if (bytesRead < 0)
            return loadErrorPage(loc, 500);
        resp.setBody(std::move(buffer));
        resp.setStatusCode(200);
        resp.setReasonPhrase("OK");
        resp.addHeader("Content-Type", MimeTypes::detectMimeType(fullPath));
    }

    // Headers
    resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
    resp.addHeader("Connection", "close");
    resp.addHeader("Date", Utils::currentDateString());
    resp.addHeader("Server", Utils::serverNameString());
    return resp;
}

Response StaticHandler::loadErrorPage(const Location& loc, int code) {
    Response resp;
    resp.setStatusCode(code);
    switch (code) {
        case 403: resp.setReasonPhrase("Forbidden"); break;
        case 404: resp.setReasonPhrase("Not Found"); break;
        case 405: resp.setReasonPhrase("Method Not Allowed"); break;
        case 500: resp.setReasonPhrase("Internal Server Error"); break;
        default: resp.setReasonPhrase("Error");
    }
    auto it = loc._errorPages.find(code);
    if (it != loc._errorPages.end()) {
        std::string errorPage = loc._root + it->second;
        struct stat sb;
        if (stat(errorPage.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
            int fd = open(errorPage.c_str(), O_RDONLY);
            if (fd >= 0) {
                std::vector<char> buffer(sb.st_size);
                ssize_t bytesRead = read(fd, buffer.data(), sb.st_size);
                close(fd);
                if (bytesRead >= 0) {
                    resp.setBody(std::move(buffer));
                    resp.addHeader("Content-Type", MimeTypes::detectMimeType(errorPage));
                    resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
                }
            }
        }
    }
    return resp;
}

std::vector<char> StaticHandler::generateDirectoryListing(const std::string& fsPath, const std::string& urlPath) {
    std::ostringstream html;
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1><ul>";

    DIR* dir = opendir(fsPath.c_str());
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        html << "<li><a href=\"" << name << "\">" << name << "</a></li>";
    }
    closedir(dir);

    html << "</ul></body></html>";
    std::string body = html.str();
    return std::vector<char>(body.begin(), body.end());
}