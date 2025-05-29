#include "StaticHandler.hpp"

StaticHandler::StaticHandler() {}

StaticHandler::~StaticHandler() {}

Response StaticHandler::serveGet(const HTTPRequest& req, const Location& loc) {
    // 1) Method must be allowed
    if (std::find(loc._locationAllowedMethods.begin(), loc._locationAllowedMethods.end(),
                  "GET") == loc._locationAllowedMethods.end()) {
        return loadErrorPage(loc, 405);
    }

    std::string uri;
    if (!loc._locationReturnPages.empty()) {
        uri = loc._locationReturnPages;
    } else {
        // 2) Build the filesystem path
        uri = req.getUri();                   // e.g. "/files" or "/files/"
    }
    // strip the location prefix
    std::string rel = uri.substr(loc._locationPath.length()); // e.g. "files" or "files/"
    if (rel.empty()) rel = "/";                       // treat "/" uniformly

    // normalize root with trailing slash
    std::string fsRoot = loc._locationRoot;
    if (fsRoot.back() != '/') fsRoot += '/';

    // normalize rel to never start with slash
    if (rel.front() == '/') rel.erase(0,1);

    std::string fullPath = fsRoot + rel;              // e.g. "./www/files"

    struct stat sb;
    // 3) Does it exist?
    if (stat(fullPath.c_str(), &sb) < 0) {
        // nothing at that path → 404
        return loadErrorPage(loc, 404);
    }

    // 4) Directory case
    if (S_ISDIR(sb.st_mode)) {
        // a) redirect if missing trailing slash
        if (uri.back() != '/') {
            Response r;
            r.setStatusCode(301);
            r.setReasonPhrase("Moved Permanently");
            r.addHeader("Location", uri + "/");
            return r;
        }

        // b) try index file
        std::string idxPath = fullPath;
        if (idxPath.back() != '/') idxPath += '/';
        idxPath += loc._locationIndex;                       // e.g. "./www/files/index.html"

        struct stat sb2;
        if (stat(idxPath.c_str(), &sb2) == 0 && S_ISREG(sb2.st_mode)) {
            // serve the index.html
            int fd = open(idxPath.c_str(), O_RDONLY);
            if (fd < 0) return loadErrorPage(loc, 404);
            std::vector<char> buf(sb2.st_size);
            if (read(fd, buf.data(), sb2.st_size) < 0) {
                close(fd);
                return loadErrorPage(loc, 500);
            }
            close(fd);

            Response resp;
            resp.setStatusCode(200);
            resp.setReasonPhrase("OK");
            resp.setBody(std::move(buf));
            resp.addHeader("Content-Type", MimeTypes::detectMimeType(idxPath));
            // fall through to stamp headers below
            resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
            resp.addHeader("Connection", "close");
            resp.addHeader("Date", Utils::currentDateString());
            resp.addHeader("Server", Utils::serverNameString());
            return resp;
        }

        // c) no index, auto-index?
        if (loc._locationAutoIndex) {
            auto listing = generateDirectoryListing(fullPath, uri);
            Response resp;
            resp.setStatusCode(200);
            resp.setReasonPhrase("OK");
            resp.setBody(std::move(listing));
            resp.addHeader("Content-Type", "text/html");
            resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
            resp.addHeader("Connection", "close");
            resp.addHeader("Date", Utils::currentDateString());
            resp.addHeader("Server", Utils::serverNameString());
            return resp;
        }

        // d) no index & no auto-index → forbidden
        return loadErrorPage(loc, 403);
    }

    // 5) File case
    if (S_ISREG(sb.st_mode)) {
        int fd = open(fullPath.c_str(), O_RDONLY);
        if (fd < 0) return loadErrorPage(loc, 404);
        std::vector<char> buf(sb.st_size);
        if (read(fd, buf.data(), sb.st_size) < 0) {
            close(fd);
            return loadErrorPage(loc, 500);
        }
        close(fd);

        Response resp;
        resp.setStatusCode(200);
        resp.setReasonPhrase("OK");
        resp.setBody(std::move(buf));
        resp.addHeader("Content-Type", MimeTypes::detectMimeType(fullPath));
        resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));
        resp.addHeader("Connection", "close");
        resp.addHeader("Date", Utils::currentDateString());
        resp.addHeader("Server", Utils::serverNameString());
        return resp;
    }

    // 6) All other filesystem objects → treat as not found
    return loadErrorPage(loc, 404);
}

Response StaticHandler::serve(const HTTPRequest& req, const Location& loc) {
    const auto& method = req.getMethod();
    if (method == "GET") {
        return serveGet(req, loc);
    }
    
    Response resp;
    resp.setStatusCode(405);
    resp.setReasonPhrase("Method Not Allowed");
    std::ostringstream allow;
    for (size_t i = 0; i < loc._locationAllowedMethods.size(); i++) {
        if (i) allow << ", ";
        allow << loc._locationAllowedMethods[i];
    }
    resp.addHeader("Allow", allow.str());
    return loadErrorPage(loc, 405);
}

Response StaticHandler::loadErrorPage(const Location& loc, int code) {
    Response resp;
    resp.setStatusCode(code);
    switch (code) {
        case 400: resp.setReasonPhrase("Bad Request"); break;
        case 403: resp.setReasonPhrase("Forbidden"); break;
        case 404: resp.setReasonPhrase("Not Found"); break;
        case 405: resp.setReasonPhrase("Method Not Allowed"); break;
        case 409: resp.setReasonPhrase("Conflict"); break;
        case 413: resp.setReasonPhrase("Payload Too Large"); break;
        case 500: resp.setReasonPhrase("Internal Server Error"); break;
        default: resp.setReasonPhrase("Error");
    }
    auto it = loc._locationErrorPages.find(code);
    if (it != loc._locationErrorPages.end()) {
        std::string errorPage = loc._locationRoot + it->second;
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