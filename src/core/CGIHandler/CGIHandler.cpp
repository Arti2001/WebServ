#include "CGIHandler.hpp"

Response CGIHandler::handle(const HTTPRequest& req) {
    try {
        if (!isCGIRequest(req.getUri())) {
            throw CGIException("Not a CGI request");
        }

        //resolve script path
        std::string scriptPath = resolveScriptPath(req.getUri());

        //execute script
        std::vector<char> output = executeScript(req, scriptPath);

        //parse output
        return parseOutput(output);
    } catch (const CGIException& e) {
        //create an error response
        Response errorResponse;
        errorResponse.setStatusCode(500);
        errorResponse.setReasonPhrase("Internal Server Error");

        std::string errorMsg = "CGI Error: " + std::string(e.what());
        std::vector<char> body(errorMsg.begin(), errorMsg.end());
        errorResponse.setBody(body);

        return errorResponse;
    }
}

bool CGIHandler::isCGIRequest(const std::string& uri) {
    return uri.find("/cgi-bin/") != std::string::npos;
}

std::string CGIHandler::resolveScriptPath(const std::string& uri) {
    // extract the script path from uri
    std::string scriptPath = uri;

    // remove any query parameters
    size_t questionMarkPos = scriptPath.find('?');
    if (questionMarkPos != std::string::npos) {
        scriptPath = scriptPath.substr(0, questionMarkPos);
    }

    //for now convert /cgi-bin/script.py to ./myWebsite/cgi-bin/script.py
    // might need to adjust this later TODO
    if (scriptPath.find("/cgi-bin/") == 0) {
        scriptPath = "./myWebsite" + scriptPath;
    }
    
    // check if the file exists
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) < 0) {
        throw CGIException("Script file not found: " + scriptPath);
    }

    // check if the file is executable
    if ((fileStat.st_mode & S_IXUSR) == 0) {
        throw CGIException("Script file is not executable: " + scriptPath);
    }

    return scriptPath;
}

std::pair<std::string, std::string> CGIHandler::extractScriptNameAndPathInfo(const std::string& uri) {
    // split the URI into script name and PATH_INFO
    size_t cgiPos = uri.find("/cgi-bin/");
    if (cgiPos == std::string::npos) {
        return std::make_pair(uri, "");
    }

    // find the start of PATH_INFO
    size_t scriptStart = cgiPos + 9;
    size_t scriptEnd = uri.find('/', scriptStart);

    std::string scriptName;
    std::string pathInfo = "";

    if (scriptEnd == std::string::npos) {
        scriptName = uri;
    } else {
        scriptName = uri.substr(0, scriptEnd);
        pathInfo = uri.substr(scriptEnd);
    }

    // remove query string from scriptName
    size_t questionMarkPos = scriptName.find('?');
    if (questionMarkPos != std::string::npos) {
        scriptName = scriptName.substr(0, questionMarkPos);
    }

    return std::make_pair(scriptName, pathInfo);
}

std::vector<std::string> CGIHandler::buildEnvironmentStrings(const HTTPRequest& req, const std::string& scriptPath) {
    std::vector<std::string> envStrings;
    std::string uri = req.getUri();

    auto [scriptName, pathInfo] = extractScriptNameAndPathInfo(uri);

    std::string queryString = "";
    size_t questionMarkPos = uri.find('?');
    if (questionMarkPos != std::string::npos) {
        queryString = uri.substr(questionMarkPos + 1);
    }

    envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envStrings.push_back("SERVER_PROTOCOL=" + req.getVersion());
    envStrings.push_back("REQUEST_METHOD=" + req.getMethod());
    envStrings.push_back("SCRIPT_NAME=" + scriptName);
    envStrings.push_back("PATH_INFO=" + pathInfo);
    envStrings.push_back("QUERY_STRING=" + queryString);
    envStrings.push_back("SERVER_SOFTWARE=webserv/1.0");
    envStrings.push_back("REMOTE_ADDR=127.0.0.1"); //simplified for now, TODO

    //content related headers
    auto headers = req.getHeaders();
    if (headers.find("Content-Type") != headers.end()) {
        envStrings.push_back("CONTENT_TYPE=" + headers.at("Content-Type"));
    }

    if (headers.find("Content-Length") != headers.end()) {
        envStrings.push_back("CONTENT_LENGTH=" + headers.at("Content-Length"));
    }

    //HTTP headers as HTTP_
    for (const auto& header : headers) {
        std::string name = header.first;
        if (name != "Content-Type" && name != "Content-Length") {
            std::string envName = "HTTP_";
            for (char c : name) {
                if (c == '-') {
                    envName += '_';
                } else {
                    envName += std::toupper(c);
                }
            }
            envStrings.push_back(envName + "=" + header.second);
        }
    }
    return envStrings;
}

char** CGIHandler::buildEnvironmentArray(const std::vector<std::string>& envStrings) {
    char** envArray = new char*[envStrings.size() + 1];
    for (size_t i = 0; i < envStrings.size(); i++) {
        envArray[i] = strdup(envStrings[i].c_str());
    }
    envArray[envStrings.size()] = nullptr;
    return envArray;
}

void CGIHandler::freeEnvironmentArray(char** envArray) {
    if (!envArray) return;

    for (size_t i = 0; envArray[i] != NULL; i++) {
        free(envArray[i]);
    }
    delete[] envArray;
}

std::string CGIHandler::getInterpreter(const std::string& scriptPath) {
    size_t dot = scriptPath.find_last_of('.');
    if (dot == std::string::npos) return "";

    // check if i actually need all of these TODO
    std::string extension = scriptPath.substr(dot);
    if (extension == ".py") return "/usr/bin/python3";
    if (extension == ".pl") return "/usr/bin/perl";
    if (extension == ".rb") return "/usr/bin/ruby";
    if (extension == ".sh") return "/bin/bash";
    if (extension == ".js") return "/usr/bin/node";
    if (extension == ".php") return "/usr/bin/php";
    return "";
}

std::vector<char> CGIHandler::executeScript(const HTTPRequest& req, const std::string& scriptPath) {

}

Response CGIHandler::parseOutput(const std::vector<char>& output) {

}