/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 12:13:08 by vshkonda          #+#    #+#             */
/*   Updated: 2025/09/17 10:48:47 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once




#include <algorithm>
#include <string_view>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <cstring>
#include <cstdio>
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
        /**
         * @brief Initializes a new CGI handler instance with request, location, and CGI index file
         * @param request The HTTP request object containing method, URI, headers, and body
         * @param location The location configuration containing root path, allowed CGI types, and upload path
         * @param cgiIndexFile Optional CGI index file to append to the script path
         * @note Automatically resolves script path, finds interpreter, and sets up environment variables
         */
        CGIHandler(const Request &request, const Location &location, std::string cgiIndexFile);

        /**
         * @brief Destroys the CGI handler and frees allocated memory
         * @return None
         * @note Automatically cleans up environment variables array
         */
        ~CGIHandler();

        // Core execution methods
        /**
         * @brief Starts the CGI process by forking, setting up pipes, and executing the script
         * @return None
         * @note Uses 3 pipes to handle stdin, stdout, and stderr
         * @throws CGIException if pipe creation or forking fails
         */
        void start();

        /**
         * @brief Handles I/O events for stdout and stderr pipes, reading output and checking process status
         * @param fd The file descriptor that triggered the event (stdout or stderr pipe)
         * @return None
         * @note Reads data in chunks and updates completion flags when pipes close
         */
        void handleEvent(int fd);

        /**
         * @brief Checks if the CGI process has completed all I/O operations
         * @return true if all operations are complete, false otherwise
         * @note Checks stdout, stderr, and process completion flags
         */
        bool isDone() const;

        /**
         * @brief Finalizes the CGI execution by parsing the output and returning the formatted response
         * @return The formatted HTTP response string
         * @note Calls parseOutput to convert raw output to proper HTTP response format
         */
        std::string finalize();

        // File descriptor getters
        /**
         * @brief Gets the stdout file descriptor for the CGI process
         * @return The stdout file descriptor
         */
        int getStdoutFd() const { return _stdout_fd; }

        /**
         * @brief Gets the stderr file descriptor for the CGI process
         * @return The stderr file descriptor
         */
        int getStderrFd() const { return _stderr_fd; }

        /**
         * @brief Gets the stdin file descriptor for the CGI process
         * @return The stdin file descriptor
         */
        int getStdinFd() const { return _stdin_fd; }

        // Exception class for CGI errors
        /**
         * @brief Exception class for CGI-specific errors with HTTP status codes
         * @details Extends std::runtime_error to provide both error message and HTTP status code
         *          for proper error handling in HTTP responses.
         */
        class CGIException : public std::runtime_error {
            public:
                /**
                 * @brief Constructs a CGI exception with message and status code
                 * @param message The error message
                 * @param statusCode The HTTP status code for the error
                 */
                CGIException(const std::string& message, int statusCode) 
                    : std::runtime_error(message), _statusCode(statusCode) {}
                
                /**
                 * @brief Gets the HTTP status code for this exception
                 * @return The HTTP status code
                 */
                int statusCode() const { return _statusCode; }
            
            private:
                int _statusCode; // HTTP status code for the error
        };

    private:
        // Environment and configuration methods
        /**
         * @brief Initializes environment variables required for CGI script execution
         * @param request The HTTP request object containing headers, method, and other request data
         * @return Map containing all required CGI environment variables
         * @note Sets standard CGI variables like REQUEST_METHOD, SCRIPT_NAME, QUERY_STRING, etc.
         */
        std::unordered_map<std::string, std::string> initEnvironmentVars(const Request& request);

        /**
         * @brief Converts environment variables map to a C-style string array for execve
         * @param envVariables Map containing environment variable names and values
         * @return Pointer to null-terminated array of environment variable strings
         * @note Allocates memory for each string and handles cleanup on allocation failure
         * @throws CGIException if memory allocation fails
         */
        char** buildEnvironmentArray(const std::unordered_map<std::string, std::string>& envVariables);

        /**
         * @brief Frees allocated memory for environment variables array
         * @return None
         * @note Safely deallocates all strings and the array pointer
         */
        void freeEnvironmentArray();

        // Script execution methods
        /**
         * @brief Determines the appropriate interpreter path based on script file extension
         * @param scriptPath The full path to the script file
         * @return The path to the interpreter executable
         * @note Check OS specific path for exec (e.g. python3 - macOS: /opt/homebrew/bin/python3; linux: /usr/bin/python3
         */
        std::string getInterpreter(const std::string& scriptPath);

        /**
         * @brief Parses CGI script output and formats it into a proper HTTP response
         * @param output Vector containing the raw output from the CGI script
         * @return The formatted HTTP response string
         * @note Handles both cases: output with headers and output without headers
         */
        std::string parseOutput(const std::vector<char>& output);

        // Path and file handling methods
        /**
         * @brief Resolves the full path to the CGI script by combining root path, URI, and index file
         * @param rootPath The root directory path for the location
         * @param uri The request URI path
         * @param cgiIndexFile Optional CGI index file name to append
         * @return The resolved absolute path to the CGI script
         * @note Validates that the script file exists and is executable
         * @throws CGIException if script file is not found (404) or not executable (403)
         */
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