/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/30 12:13:08 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:21:08 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <algorithm>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <cstring>
#include <cstdio>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

#define MAX_OUTPUT_SIZE 10 * 1024 * 1024 // 10 MB maximum output size
#define CHUNK_SIZE 8192 // 8 KB chunk size for reading/writing

/**
 * @brief CGI script execution handler for the webserv application
 * @details This class manages the execution of Common Gateway Interface (CGI) scripts, handling 
 *          process creation, inter-process communication through pipes, environment variable setup, 
 *          and output parsing. It supports various scripting languages (Python, Perl, Ruby, Bash, 
 *          Node.js, PHP) and automatically determines the appropriate interpreter based on file extensions.
 * @note The class uses non-blocking I/O for stdout and stderr pipes, supports timeout handling,
 *       and provides comprehensive error handling through custom exceptions.
 */
class CGIHandler {
    public:
        // Constructors and Destructor
        CGIHandler(const Request &request, const Location &location, std::string cgiIndexFile);
        ~CGIHandler();

        // Core execution methods
        void start();
        void handleEvent(int fd);
        bool isDone() const;
        std::string finalize();

        // File descriptor getters
        int getStdoutFd() const { return _stdout_fd; }
        int getStderrFd() const { return _stderr_fd; }
        int getStdinFd() const { return _stdin_fd; }

        // Exception class for CGI errors
        class CGIException : public std::runtime_error {
            public:
                CGIException(const std::string& message, int statusCode) 
                    : std::runtime_error(message), _statusCode(statusCode) {}
                
                int statusCode() const { return _statusCode; }
            
            private:
                int _statusCode; // HTTP status code for the error
        };

    private:
        // Environment and configuration methods
        std::unordered_map<std::string, std::string> initEnvironmentVars(const Request& request);
        char** buildEnvironmentArray(const std::unordered_map<std::string, std::string>& envVariables);
        void freeEnvironmentArray();

        // Script execution methods
        std::string getInterpreter(const std::string& scriptPath);
        std::string parseOutput(const std::vector<char>& output);

        // Path and file handling methods
        std::string resolveScriptPath(const std::string& rootPath, const std::string& uri, const std::string& cgiIndexFile);

        // Request and script attributes
        const Request& _request; // Reference to the HTTP request object
        std::string _interpreter; // Interpreter for the CGI script (e.g., Python, Perl)
        std::string _cgiPath; // Path to the CGI script interpreter
        std::string _scriptPath; // Full path to the CGI script file
        std::string _queryString; // Query string for the CGI script
        std::string _bodyInput; // Body input for the CGI script, if applicable
        std::string _cgiUploadPath; // Upload directory path for CGI scripts
        time_t _timeout; // Timeout for the CGI script execution

        // Process and pipe attributes
        int _stdin_fd; // File descriptor for the CGI script's stdin
        int _stdout_fd; // File descriptor for the CGI script's stdout
        int _stderr_fd; // File descriptor for the CGI script's stderr
        pid_t _pid; // Process ID of the CGI script

        // Execution state attributes
        bool _stdout_done; // Flag to indicate if stdout is done reading
        bool _stderr_done; // Flag to indicate if stderr is done reading
        bool _process_done; // Flag to indicate if the CGI process has finished

        // Output storage attributes
        char** _envp; // Environment variables array for execve
        std::vector<char> _output; // Output from the CGI script
        std::vector<char> _errorOutput; // Error output from the CGI script
};