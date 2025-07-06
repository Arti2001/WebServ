/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIHandler.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/30 12:13:01 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/07/06 11:44:38 by vshkonda      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"


CGIHandler::CGIHandler(const Request &request, const Location &location, std::string cgiIndexFile) 
    : _request(request), _envp(nullptr), _interpreter("") , _cgiPath(""), _queryString(""), _bodyInput(""), _cgiUploadPath("") {
    // Initialize the CGI handler with the response
    // This constructor can be used to set up any initial state if needed
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
	std::unordered_map<std::string, std::string> envVariables = initEnvironmentVars(request);
    _envp = buildEnvironmentArray(envVariables);
	_timeout = request.getTimeout();
	_stderr_done = false;
	_stdout_done = false;
	_process_done = false;
    }

CGIHandler::~CGIHandler()
{
	freeEnvironmentArray();
}
	
std::string CGIHandler::resolveScriptPath(const std::string& rootPath, const std::string& uri, const std::string& cgiIndexFile)
{
    std::string scriptPath;
    scriptPath = joinPaths(rootPath, uri);
    if (!cgiIndexFile.empty())
        scriptPath = joinPaths(scriptPath, cgiIndexFile);

    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) < 0) {
        throw CGIException("Script file not found: " + scriptPath, 404);
    }

    // check if the file is executable
    if ((fileStat.st_mode & S_IXUSR) == 0) {
        throw CGIException("Script file is not executable: " + scriptPath, 403);
    }
    return scriptPath;
}

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
    // Register _stdout_fd and _stderr_fd with your event loop for EPOLLIN
}

void CGIHandler::handleEvent(int fd) {
    char buffer[CHUNK_SIZE];
    ssize_t bytesRead;
    if (fd == _stdout_fd && !_stdout_done) {
        while ((bytesRead = read(_stdout_fd, buffer, sizeof(buffer))) > 0) {
            _output.insert(_output.end(), buffer, buffer + bytesRead);
        }
        if (bytesRead == 0) { // EOF
            close(_stdout_fd);
            _stdout_done = true;
        }
    }
    if (fd == _stderr_fd && !_stderr_done) {
        while ((bytesRead = read(_stderr_fd, buffer, sizeof(buffer))) > 0) {
            _errorOutput.insert(_errorOutput.end(), buffer, buffer + bytesRead);
        }
        if (bytesRead == 0) { // EOF
            close(_stderr_fd);
            _stderr_done = true;
        }
    }
    int status;
    if (!_process_done && waitpid(_pid, &status, WNOHANG) > 0) {
        _process_done = true;
    }
}

bool CGIHandler::isDone() const {
    return _stdout_done && _stderr_done && _process_done;
}

std::string CGIHandler::finalize() {
    return parseOutput(_output);
}

std::unordered_map<std::string, std::string> CGIHandler::initEnvironmentVars(const Request& request) {
    
    std::unordered_map<std::string, std::string> envVariables;
    // Initialize environment variables for CGI execution
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
    // Add more environment variables as needed
    return envVariables;
}

char** CGIHandler::buildEnvironmentArray(const std::unordered_map<std::string, std::string>& envVariables) {
    char** envp = new char*[envVariables.size() + 1];
    
    size_t i = 0;
    try {
        for (const auto &pair : envVariables) {
            std::string envVar = pair.first + "=" + pair.second;
            // Allocate each string with new[]
            envp[i] = new char[envVar.length() + 1];
            // Copy string data
            std::strcpy(envp[i], envVar.c_str());
            ++i;
        }
        envp[i] = nullptr; // Null-terminate array
    } catch (...) {
        // Cleanup if any allocation fails
        for (size_t j = 0; j < i; ++j) {
            delete[] envp[j];
        }
        delete[] envp;
        throw CGIException("Failed to allocate memory for environment variables", 500);
    }
    return envp;
}

void CGIHandler::freeEnvironmentArray() {
    if (!_envp) return;

    for (size_t i = 0; _envp[i] != nullptr; i++) {
        delete[] _envp[i];
    }
    
    delete[] _envp;
    _envp = nullptr; 
}


std::string CGIHandler::getInterpreter(const std::string& scriptPath) {
    size_t dot = scriptPath.find_last_of('.');
    if (dot == std::string::npos) return "";
    // check if i actually need all of these TODO
    std::string extension = scriptPath.substr(dot);
    _interpreter = extension;
    if (extension == ".py") return "/usr/bin/python3"; // for macOS is /opt/homebrew/bin/python3 for linux is /usr/bin/python3
    if (extension == ".pl") return "/usr/bin/perl";
    if (extension == ".rb") return "/usr/bin/ruby";
    if (extension == ".sh") return "/bin/bash";
    if (extension == ".js") return "/usr/bin/node";
    if (extension == ".php") return "/usr/bin/php";
    return "";
}

std::string CGIHandler::parseOutput(const std::vector<char>& output) {
    std::string rawResponse;
    std::string startLine = "HTTP/1.1 200 OK\r\n";
    rawResponse += startLine;

    std::string outputStr(output.begin(), output.end());
    // separate any headers that might be in the output
    std::string headers;
    //find separator between headers and body
    size_t headerEnd = outputStr.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
       //no headers, treat output as body
       headers += "Content-Type: text/html\r\n";
       headers += "\r\nContent-Length: " + std::to_string(output.size()) + "\r\n";
       rawResponse += headers + "\r\n";
       rawResponse += outputStr;
       return rawResponse;
    }
    //extract headers
    headers = outputStr.substr(0, headerEnd);
    //extract body
    std::string body = outputStr.substr(headerEnd + 4); // +4 to skip the \r\n\r\n
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


std::string CGIHandler::joinPaths(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    
    if (path1.back() == '/' && path2.front() == '/') {
        return path1 + path2.substr(1);
    } else if (path1.back() == '/' || path2.front() == '/') {
        return path1 + path2;
    } else {
        return path1 + "/" + path2;
    }
}