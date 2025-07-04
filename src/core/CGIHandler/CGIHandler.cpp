/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIHandler.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/30 12:13:01 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/07/04 12:47:33 by vshkonda      ########   odam.nl         */
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
        throw CGIException("No interpreter found for script: " + _scriptPath);
    }
    if (location.getLocationAllowedCgi().find(_interpreter) == location.getLocationAllowedCgi().end()) {
        throw CGIException("Selected interpreter unavailable for this location: " + _scriptPath);
    }
    _queryString = request.getQuery();
    _bodyInput = request.getBody();
	_cgiUploadPath = location.getLocationUploadPath();
	std::unordered_map<std::string, std::string> envVariables = initEnvironmentVars(request);
    _envp = buildEnvironmentArray(envVariables);
	_timeout = request.getTimeout();
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
        throw CGIException("Script file not found: " + scriptPath);
    }

    // check if the file is executable
    if ((fileStat.st_mode & S_IXUSR) == 0) {
        throw CGIException("Script file is not executable: " + scriptPath);
    }
    return scriptPath;
}

 std::string CGIHandler::process() {
    std::vector<char> output = executeScript(_request);
    std::string result = parseOutput(output);
    std::cout << "CGI output: " << result << std::endl;
	return result;
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
        throw std::runtime_error("Failed to allocate memory for environment variables");
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

std::vector<char> CGIHandler::executeScript(const Request& req) {
    //setup pipes for communication
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0) {
        throw CGIException("Failed to create pipes");
    }
	pid_t pid = fork();
    if (pid < 0) {
        //fork failed
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        throw CGIException("Failed to fork");
    }
    if (pid == 0) {
		std::string scriptName;
        //child process
        //redirect stdin/stdout/stderr
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        
        // Close all original pipe file descriptors as they are duplicated now
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        
        // Change to script directory
        size_t lastSlash = _scriptPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            std::string scriptDir = _scriptPath.substr(0, lastSlash);
            if (chdir(scriptDir.c_str()) != 0) {
                perror("CGI chdir failed");
                exit(1);
            }
        }

        // Extract script filename
    	if (lastSlash != std::string::npos) 
    	     scriptName = _scriptPath.substr(lastSlash + 1);
        if (!_cgiPath.empty()) {
    		const char* args[] = {_cgiPath.c_str(), scriptName.c_str(), nullptr};
    		execve(_cgiPath.c_str(), const_cast<char* const*>(args), _envp);
		} else {
    		const char* args[] = {scriptName.c_str(), nullptr};
    		execve(scriptName.c_str(), const_cast<char* const*>(args), _envp);
		}
		
        //execve failed if we get here
        perror("CGI execve failed for script");
        exit(1);
    }
    
    //close unused pipes
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);
	//send request body to script stdin (for POST)

    if (req.getMethod() == "POST") {
        if (!_bodyInput.empty()) {
            	write(stdin_pipe[1], _bodyInput.c_str(), _bodyInput.size());
		}
	}
    //close stdin to signal EOF
    close(stdin_pipe[1]);
    // Read output from the child process
    return readFromPipes(stdout_pipe[0], stderr_pipe[0], pid);
}

std::vector<char> CGIHandler::readFromPipes(int stdout_fd, int stderr_fd, pid_t pid) {
    std::vector<char> output;
    std::vector<char> errorOutput;
    char buffer[CHUNK_SIZE];
    ssize_t bytesRead;

    fd_set read_fds;
    struct timeval timeout;

    while (stdout_fd != -1 || stderr_fd != -1) {
        FD_ZERO(&read_fds);
        if (stdout_fd != -1) FD_SET(stdout_fd, &read_fds);
        if (stderr_fd != -1) FD_SET(stderr_fd, &read_fds);

        int max_fd = 0;
        if (stdout_fd != -1) max_fd = std::max(max_fd, stdout_fd);
        if (stderr_fd != -1) max_fd = std::max(max_fd, stderr_fd);

        timeout.tv_sec = _timeout;
        timeout.tv_usec = 0;

        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);

		std::cout << "stdout_fd: " << stdout_fd << ", stderr_fd: " << stderr_fd << std::endl;
        if (activity < 0) {
            if (stdout_fd != -1) close(stdout_fd);
            if (stderr_fd != -1) close(stderr_fd);
            throw CGIException("select() failed", 500);
        }
		std::cout << "CGI select activity: " << activity << std::endl;
        if (activity == 0) {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            if (stdout_fd != -1) close(stdout_fd);
            if (stderr_fd != -1) close(stderr_fd);
            throw CGIException("CGI script timed out", 504);
        }

        if (stdout_fd != -1 && FD_ISSET(stdout_fd, &read_fds)) {
            bytesRead = read(stdout_fd, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                output.insert(output.end(), buffer, buffer + bytesRead);
                if (output.size() > MAX_OUTPUT_SIZE) {
                    kill(pid, SIGTERM);
                    close(stdout_fd);
                    close(stderr_fd);
                    throw CGIException("Max output size exceeded", 413);
                }
            } else {
                close(stdout_fd);
                stdout_fd = -1;
            }
        }

        if (stderr_fd != -1 && FD_ISSET(stderr_fd, &read_fds)) {
            bytesRead = read(stderr_fd, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                errorOutput.insert(errorOutput.end(), buffer, buffer + bytesRead);
            } else {
                close(stderr_fd);
                stderr_fd = -1;
            }
        }
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        //script exited with non-zero status
        std::string errorMsg = "CGI exited with non-zero status " + std::to_string(WEXITSTATUS(status));
        if (!errorOutput.empty()) {
            errorMsg += ". Stderr: " + std::string(errorOutput.begin(), errorOutput.end());
        }
        throw CGIException(errorMsg);
    }
    
    return output;
}

std::string CGIHandler::parseOutput(const std::vector<char>& output) {
    std::string rawResponse;
    std::string startLine = "HTTP/1.1 200 OK\r\n";
    rawResponse += startLine;

    std::string outputStr(output.begin(), output.end());
    std::cout << "CGI output string: " << outputStr << std::endl;
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