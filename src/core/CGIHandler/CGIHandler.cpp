/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIHandler.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: amysiv <amysiv@student.42.fr>                +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/30 12:13:01 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 20:52:25 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

/*CGIHandler constructor
/*brief Initializes a new CGI handler instance with request, location, and CGI index file
/*@param request The HTTP request object containing method, URI, headers, and body
/*@param location The location configuration containing root path, allowed CGI types, and upload path
/*@param cgiIndexFile Optional CGI index file to append to the script path
*/
CGIHandler::CGIHandler(const Request &request, const Location &location, std::string cgiIndexFile) 
    : _request(request), _envp(nullptr), _interpreter("") , _cgiPath(""), _queryString(""), _bodyInput(""), _cgiUploadPath("") {
    _scriptPath = resolveScriptPath(location.getLocationRoot(), request.getUri(), cgiIndexFile);
    _cgiPath = getInterpreter(_scriptPath);
    if (_cgiPath.empty()) {
        throw CGIException("No interpreter found for script: " + _scriptPath, 500);
    }
    if (location.getLocationAllowedCgi().find(_interpreter) == location.getLocationAllowedCgi().end()) {
        throw CGIException("Selected interpreter unavailable for this location: " + _scriptPath, 500);
    }
    _queryString = request.getQuery();
    _bodyInput = request.getBody();
	_cgiUploadPath = location.getLocationUploadPath();
	_timeout = request.getTimeout();
	std::unordered_map<std::string, std::string> envVariables = initEnvironmentVars(request);
    _envp = buildEnvironmentArray(envVariables);

	_stderr_done = false;
	_stdout_done = false;
	_process_done = false;
    }

/*CGIHandler destructor
/*brief Cleans up allocated memory for environment variables
/*@param None
*/
CGIHandler::~CGIHandler()
{
	freeEnvironmentArray();
}

/*resolveScriptPath
/*brief Resolves the full path to the CGI script by combining root path, URI, and index file
/*@param rootPath The root directory path for the location
/*@param uri The request URI path
/*@param cgiIndexFile Optional CGI index file name to append
*/
std::string CGIHandler::resolveScriptPath(const std::string& rootPath, const std::string& uri, const std::string& cgiIndexFile)
{
    std::string scriptPath;
    scriptPath = Utils::joinPaths(rootPath, uri);
    if (!cgiIndexFile.empty())
        scriptPath = Utils::joinPaths(scriptPath, cgiIndexFile);

    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) < 0) {
        throw CGIException("Script file not found: " + scriptPath, 404);
    }

    if ((fileStat.st_mode & S_IXUSR) == 0) {
        throw CGIException("Script file is not executable: " + scriptPath, 403);
    }
    return scriptPath;
}

/*start
/*brief Starts the CGI process by forking, setting up pipes, and executing the script
/*@param None
/*note: we use 3 pipes to handle stdin, stdout, and stderr
*/
void CGIHandler::start() {
    int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0)
        throw CGIException("Failed to create pipes", 500);

    _pid = fork();
    if (_pid < 0) throw CGIException("Failed to fork", 500);

    if (_pid == 0) {
        std::string scriptName;
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdin_pipe[0]); 
		close(stdin_pipe[1]);
        close(stdout_pipe[0]); 
		close(stdout_pipe[1]);
        close(stderr_pipe[0]); 
		close(stderr_pipe[1]);
        size_t lastSlash = _scriptPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            std::string scriptDir = _scriptPath.substr(0, lastSlash);
            if (chdir(scriptDir.c_str()) != 0) {
                perror("CGI chdir failed");
                exit(1);
            }
        }
        if (lastSlash != std::string::npos)
            scriptName = _scriptPath.substr(lastSlash + 1);
        if (!_cgiPath.empty()) {
            const char* args[] = {_cgiPath.c_str(), scriptName.c_str(), nullptr};
            execve(_cgiPath.c_str(), const_cast<char* const*>(args), _envp);
        } else {
            const char* args[] = {scriptName.c_str(), nullptr};
            execve(scriptName.c_str(), const_cast<char* const*>(args), _envp);
        }
        perror("CGI execve failed for script");
        exit(1);
    }

    _stdin_fd = stdin_pipe[1];
    _stdout_fd = stdout_pipe[0];
    _stderr_fd = stderr_pipe[0];
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);
	ServerManager::setNonBlocking(_stdout_fd);
	ServerManager::setNonBlocking(_stderr_done);
    _stdout_done = false;
    _stderr_done = false;
    _process_done = false;
    if (_request.getMethod() == "POST" && !_bodyInput.empty()) {
        write(_stdin_fd, _bodyInput.c_str(), _bodyInput.size());
    }
    close(_stdin_fd);
}

/*handleEvent
/*brief Handles I/O events for stdout and stderr pipes, reading output and checking process status
/*@param fd The file descriptor that triggered the event (stdout or stderr pipe)
*/
void CGIHandler::handleEvent(int fd) {
    char buffer[CHUNK_SIZE];
    ssize_t bytesRead;
    if (fd == _stdout_fd && !_stdout_done) {
        while ((bytesRead = read(_stdout_fd, buffer, sizeof(buffer))) > 0) {
            _output.insert(_output.end(), buffer, buffer + bytesRead);
        }
        if (bytesRead == 0) {
            close(_stdout_fd);
            _stdout_done = true;
        }
    }
    if (fd == _stderr_fd && !_stderr_done) {
        while ((bytesRead = read(_stderr_fd, buffer, sizeof(buffer))) > 0) {
            _errorOutput.insert(_errorOutput.end(), buffer, buffer + bytesRead);
        }
        if (bytesRead == 0) {
            close(_stderr_fd);
            _stderr_done = true;
        }
    }
    int status;
    if (!_process_done && (waitpid(_pid, &status, WNOHANG) > 0 || WIFEXITED(status))) {
        _process_done = true;
    }
}

/*isDone
/*brief Checks if the CGI process has completed all I/O operations
/*@param None
*/
bool CGIHandler::isDone() const {
    return _stdout_done && _stderr_done && _process_done;
}

/*finalize
/*brief Finalizes the CGI execution by parsing the output and returning the formatted response
/*@param None
*/
std::string CGIHandler::finalize() {
    return parseOutput(_output);
}

/*initEnvironmentVars
/*brief Initializes environment variables required for CGI script execution
/*@param request The HTTP request object containing headers, method, and other request data
*/
std::unordered_map<std::string, std::string> CGIHandler::initEnvironmentVars(const Request& request) {
    
    std::unordered_map<std::string, std::string> envVariables;
    envVariables["REQUEST_METHOD"] = request.getMethod();
    envVariables["SCRIPT_NAME"] = _scriptPath;
    envVariables["QUERY_STRING"] = _queryString;
	auto it = request.getHeaders().find("Content-Type");
	envVariables["CONTENT_TYPE"] = (it != request.getHeaders().end()) ? it->second : Response::getMimeType(_scriptPath);
    envVariables["CONTENT_LENGTH"] = std::to_string(request.getBodySize());
    envVariables["SERVER_NAME"] = request.getHeaders().at("Host");
    envVariables["SERVER_PROTOCOL"] = request.getHttpVersion();
    envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
	envVariables["UPLOAD_DIR"] = _cgiUploadPath;
	envVariables["TIMEOUT"] = std::to_string(_timeout);
    return envVariables;
}

/*buildEnvironmentArray
/*brief Converts environment variables map to a C-style string array for execve
/*@param envVariables Map containing environment variable names and values
*/
char** CGIHandler::buildEnvironmentArray(const std::unordered_map<std::string, std::string>& envVariables) {
    char** envp = new char*[envVariables.size() + 1];
    
    size_t i = 0;
    try {
        for (const auto &pair : envVariables) {
            std::string envVar = pair.first + "=" + pair.second;
            envp[i] = new char[envVar.length() + 1];
            std::strcpy(envp[i], envVar.c_str());
            ++i;
        }
        envp[i] = nullptr;
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            delete[] envp[j];
        }
        delete[] envp;
        throw CGIException("Failed to allocate memory for environment variables", 500);
    }
    return envp;
}

/*freeEnvironmentArray
/*brief Frees allocated memory for environment variables array
/*@param None
*/
void CGIHandler::freeEnvironmentArray() {
    if (!_envp) return;

    for (size_t i = 0; _envp[i] != nullptr; i++) {
        delete[] _envp[i];
    }
    
    delete[] _envp;
    _envp = nullptr; 
}

/*getInterpreter
/*brief Determines the appropriate interpreter path based on script file extension
/*@param scriptPath The full path to the script file
/*note check OS specific path for exec (e.g. python3 - macOS: /opt/homebrew/bin/python3; linux: /usr/bin/python3
*/
std::string CGIHandler::getInterpreter(const std::string& scriptPath) {
    size_t dot = scriptPath.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string extension = scriptPath.substr(dot);
    _interpreter = extension;
    if (extension == ".py") return "/usr/bin/python3";
    if (extension == ".pl") return "/usr/bin/perl";
    if (extension == ".rb") return "/usr/bin/ruby";
    if (extension == ".sh") return "/bin/bash";
    if (extension == ".js") return "/usr/bin/node";
    if (extension == ".php") return "/usr/bin/php";
    return "";
}

/*parseOutput
/*brief Parses CGI script output and formats it into a proper HTTP response
/*@param output Vector containing the raw output from the CGI script
*/
std::string CGIHandler::parseOutput(const std::vector<char>& output) {
    std::string rawResponse;
    std::string startLine = "HTTP/1.1 200 OK\r\n";
    rawResponse += startLine;

    std::string outputStr(output.begin(), output.end());
    std::string headers;
    size_t headerEnd = outputStr.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
       headers += "Content-Type: text/html\r\n";
       headers += "Content-Length: " + std::to_string(output.size()) + "\r\n";
       rawResponse += headers + "\r\n";
       rawResponse += outputStr;
       return rawResponse;
    }
    headers = outputStr.substr(0, headerEnd);
    std::string body = outputStr.substr(headerEnd + 4);
    if (body.empty()) {
        rawResponse += headers + "\r\n";
        rawResponse += "Content-Length: 0\r\n\r\n";
        return rawResponse;
    }
    headers += "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n";
    rawResponse += headers + "\r\n";
	rawResponse += body;

    return rawResponse;
}


