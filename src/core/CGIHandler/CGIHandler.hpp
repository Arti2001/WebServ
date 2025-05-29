#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include "../parsingRequest/HTTPRequest.hpp"
#include "../parsingResponse/Response.hpp"

class CGIHandler {
    public:
        CGIHandler() = default;
        ~CGIHandler() = default;

        Response handle(const HTTPRequest& req);

        static bool isCGIRequest(const std::string &uri);

        class CGIException : public std::runtime_error {
            public:
                CGIException(const std::string& message) : std::runtime_error(message) {};
        };
    
    private:
    static const int TIMEOUT_SECONDS = 30;
    static const size_t MAX_OUTPUT_SIZE = 10 * 1024 * 1024;
        //Helper methods
        std::vector<std::string> buildEnvironmentStrings(const HTTPRequest& req, const std::string& scriptPath);
        char** buildEnvironmentArray(const std::vector<std::string>& envStrings);
        std::string getInterpreter(const std::string& scriptPath);
        void freeEnvironmentArray(char** envArray);

        //execution methods
        std::vector<char> executeScript(const HTTPRequest& req, const std::string& scriptPath);
        Response parseOutput(const std::vector<char>& output);

        //path handling
        std::string resolveScriptPath(const std::string& uri);
        std::pair<std::string, std::string> extractScriptNameAndPathInfo(const std::string& uri);
};