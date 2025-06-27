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
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

#define TIMEOUT_SECONDS 5
#define MAX_OUTPUT_SIZE  10 * 1024 * 1024 // 10 MB



class CGIHandler {
    public:
        CGIHandler() = default;
        CGIHandler(const Request &Request, const Location &Location);
        ~CGIHandler() = default;

        std::string execute(void);
        class CGIException : public std::runtime_error {
            public:
                CGIException(const std::string& message) : std::runtime_error(message) {};
        };
    
    private:
        //Helper methods
        std::unordered_map<std::string, std::string> initEnvironmentVars(const Request& request);
        char** buildEnvironmentArray(const std::unordered_map<std::string, std::string>& envVariables);
        std::string getInterpreter(const std::string& scriptPath);
        void freeEnvironmentArray(char** envArray);

        //execution methods
        std::vector<char> executeScript(const Request& req, const std::string& scriptPath);
        std::string parseOutput(const std::vector<char>& output);

        //path handling
        std::string resolveScriptPath(const std::string& uri);
        std::pair<std::string, std::string> extractScriptNameAndPathInfo(const std::string& uri);

        //attributes
        char **_envp; // Environment variables for CGI execution
        std::string _interpreter; // Interpreter for the CGI script (e.g., Python, Perl)
        std::string _cgiPath; // Path to the CGI script
        std::string _scriptName; // Name of the CGI script
        std::string _queryString; // Query string for the CGI script
        std::string _bodyInput; // Body input for the CGI script, if applicable
        std::string _cgiOutput; // Output from the CGI script execution
};