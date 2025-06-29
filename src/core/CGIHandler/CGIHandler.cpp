#include "CGIHandler.hpp"
#include <sys/select.h>
#include <unistd.h>

CGIHandler::CGIHandler(const Request &request, const Location &location, std::string cgiIndexFile) 
    : _request(request), _envp(nullptr), _interpreter("") , _cgiPath(""), _queryString(""), _bodyInput(""), _cgiOutput("") {
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
    std::cout << "creating env vars" << std::endl;
    std::unordered_map<std::string, std::string> envVariables;
    envVariables = initEnvironmentVars(request);
    std::cout << "building env vars" << std::endl;
    _envp = buildEnvironmentArray(envVariables);
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
    std::cout << "starting script execution" << std::endl;
    std::vector<char> output = executeScript(_request);
    // parse output
    return parseOutput(output);
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

std::unordered_map<std::string, std::string> CGIHandler::initEnvironmentVars(const Request& request) {
    
    std::unordered_map<std::string, std::string> envVariables;
    // Initialize environment variables for CGI execution
    envVariables["REQUEST_METHOD"] = request.getMethod();
    envVariables["SCRIPT_NAME"] = _scriptPath;
    envVariables["QUERY_STRING"] = _queryString;
    envVariables["CONTENT_TYPE"] = Response::getMimeType(_scriptPath);
    envVariables["CONTENT_LENGTH"] = std::to_string(request.getBodySize());
    envVariables["SERVER_NAME"] = request.getHeaders().at("Host");
    envVariables["SERVER_PROTOCOL"] = request.getHttpVersion();
    envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
    // Add more environment variables as needed
    return envVariables;
}

char** CGIHandler::buildEnvironmentArray(const std::unordered_map<std::string, std::string>& envVariables) {
    char **envp = static_cast<char **>(malloc((envVariables.size() + 1) * sizeof(char *)));
    if (!envp) {
        throw std::runtime_error("Failed to allocate memory for environment variables");
    }

    size_t i = 0;
    for (const auto &pair : envVariables) {
        std::string envVar = pair.first + "=" + pair.second;
        envp[i] = strdup(envVar.c_str());
        if (!envp[i]) {
            throw std::runtime_error("Failed to duplicate environment variable string");
        }
        ++i;
    }
    envp[i] = nullptr; // Null-terminate the array
    return envp;
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
    _interpreter = extension;
    if (extension == ".py") return "/opt/homebrew/bin/python3"; // for macOS is /opt/homebrew/bin/python3 for linux is /usr/bin/python3
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
    std::cout << "creating child process" << std::endl;
    pid_t pid = fork();
    std::cout << "forked child process with pid: " << pid << std::endl;
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
        std::cout << "child process starting at: " << std::time(nullptr) << std::endl;
        std::cout << "executing script: " << _scriptPath << std::endl;
        std::cout << "with interpreter: " << _cgiPath << std::endl;
        
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
            std::string scriptDir = "myWebsite/pages/";
            if (chdir(scriptDir.c_str()) != 0) {
                perror("CGI chdir failed");
                exit(1);
            }
        }
        
        // Extract script filename
        std::string scriptName = "hello.py";
        // if (lastSlash != std::string::npos) {
        //     scriptName = _scriptPath.substr(lastSlash + 1);
        // }
        
        // if (!_cgiPath.empty()) {
            char* args[] = {const_cast<char*>(_cgiPath.c_str()), const_cast<char*>(scriptName.c_str()), nullptr};
            execve(_cgiPath.c_str(), args, _envp);
        // // } else {
        //     char* args[] = {const_cast<char*>(scriptName.c_str()), nullptr};
        //     execve(scriptName.c_str(), args, _envp);
        // // }

        //execve failed if we get here
        perror("CGI execve failed for script");
        freeEnvironmentArray(_envp);
        exit(1);
    }

    std::cout << "parent process starting at: " << std::time(nullptr) << std::endl;
    //parent process
    
    // Give child process time to start
    usleep(10000); // 10ms delay
    
    //close unused pipes
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);
    std::cout << "closed unused pipes" << std::endl;
    //send request body to script stdin (for POST)
    if (req.getMethod() == "POST") {
        std::string body = req.getBody();
        if (!body.empty()) {
            write(stdin_pipe[1], body.c_str(), body.length());
        }
    }
    
    //close stdin to signal EOF
    close(stdin_pipe[1]);
    std::cout << "closed stdin to signal EOF" << std::endl;
    // Read output from the child process
    std::cout << "reading from pipes" << std::endl;
    return readFromPipes(stdout_pipe[0], stderr_pipe[0], pid);
}

std::vector<char> CGIHandler::readFromPipes(int stdout_fd, int stderr_fd, pid_t pid) {
    std::vector<char> output;
    std::vector<char> errorOutput;
    char buffer[4096];
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

        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            if (stdout_fd != -1) close(stdout_fd);
            if (stderr_fd != -1) close(stderr_fd);
            throw CGIException("select() failed");
        }

        if (activity == 0) {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            if (stdout_fd != -1) close(stdout_fd);
            if (stderr_fd != -1) close(stderr_fd);
            throw CGIException("CGI script timed out");
        }

        if (stdout_fd != -1 && FD_ISSET(stdout_fd, &read_fds)) {
            bytesRead = read(stdout_fd, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                output.insert(output.end(), buffer, buffer + bytesRead);
                if (output.size() > MAX_OUTPUT_SIZE) {
                    kill(pid, SIGTERM);
                    close(stdout_fd);
                    close(stderr_fd);
                    throw CGIException("Max output size exceeded");
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

    rawResponse += "HTTP/1.1 200 OK\r\n";

    if (output.empty()) {
        rawResponse += "Content-Length: 0\r\n\r\n";
        return rawResponse;
    }
    rawResponse += "Content-Length: " + std::to_string(output.size()) + "\r\n\r\n";
    rawResponse.append(output.begin(), output.end());
    return rawResponse;

    // //convert output to string for easier parsing
    // std::string outputStr(output.begin(), output.end());

    // //find separator between headers and body
    // size_t headerEnd = outputStr.find("\r\n\r\n");
    // if (headerEnd == std::string::npos) {
    //    //no headers, treat output as body
    //    response.setBody(output);
    //    response.addHeader("Content-Type", "text/html");
    //    response.addHeader("Content-Lenght", std::to_string(output.size()));
    //    return response;
    // }

    // //extract headers
    // std::string headersStr = outputStr.substr(0, headerEnd);
    // std::istringstream headerStream(headersStr);
    // std::string line;

    // //parse each header line
    // while (std::getline(headerStream, line)) {
    //     if (line.empty() || line == "\r") continue;

    //     //remove trailing \r if present
    //     if (!line.empty() && line[line.length() -1] == '\r') {
    //         line.erase(line.length() - 1);
    //     }
    //     //find the colon separator
    //     size_t colonPos = line.find(':');
    //     if (colonPos != std::string::npos) {
    //         std::string name = line.substr(0, colonPos);
    //         std::string value = line.substr(colonPos + 1);

    //         //trim whitespace
    //         value.erase(0, value.find_first_not_of(" \t"));

    //         //check for status header
    //         if (name == "Status") {
    //             size_t spacePos = value.find(' ');
    //             if (spacePos != std::string::npos) {
    //                 int statusCode = std::stoi(value.substr(0, spacePos));
    //                 std::string reasonPhrase = value.substr(spacePos + 1);
    //                 response.setStatusCode(statusCode);
    //                 response.setReasonPhrase(reasonPhrase);
    //             }
    //         } else {
    //             response.addHeader(name, value);
    //         }
    //     }
    // }

    // //extract body
    // std::vector<char> body(output.begin() + headerEnd + 4, output.end());
    // response.setBody(body);

    // //if content type not set, default to text/html
    // if (response.getHeaders().find("Content-Type") == response.getHeaders().end()) {
    //     response.addHeader("Content-Type", "text/html");
    // }

    // //set content length header if not set
    // if (response.getHeaders().find("Content-Length") == response.getHeaders().end()) {
    //     response.addHeader("Content-Length", std::to_string(response.getBody().size()));
    // }

    // return response;
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