/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/30 12:13:08 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/06/30 12:13:17 by vshkonda      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

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
#include <sys/select.h>
#include <signal.h>
#include <cstring>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

#define TIMEOUT_SECONDS 5
#define MAX_OUTPUT_SIZE  10 * 1024 * 1024 // 10 MB



class CGIHandler {
    public:
        CGIHandler(const Request &Request, const Location &Location, std::string cgiIndexFile); // last argument needed if the request is made to the directory, so we are storing the index file that should be called with this request
        ~CGIHandler() = default;

        std::string process(void);
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
        std::vector<char> executeScript(const Request& req);
        std::string parseOutput(const std::vector<char>& output);
        std::vector<char> readFromPipes(int stdout_fd, int stderr_fd, pid_t pid);

        //path handling
        std::string resolveScriptPath(const std::string& rootPath, const std::string& uri, const std::string& cgiIndexFile);
        std::pair<std::string, std::string> extractScriptNameAndPathInfo(const std::string& uri);
        std::string joinPaths(const std::string& path1, const std::string& path2);

        //attributes
        const Request& _request;
        char **_envp; // Environment variables for CGI execution
        std::string _interpreter; // Interpreter for the CGI script (e.g., Python, Perl)
        std::string _cgiPath; // Path to the CGI script
        std::string _scriptPath; // Name of the CGI script
        std::string _queryString; // Query string for the CGI script
        std::string _bodyInput; // Body input for the CGI script, if applicable
        std::string _cgiOutput; // Output from the CGI script execution
};