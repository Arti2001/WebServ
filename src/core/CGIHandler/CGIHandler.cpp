#include "CGIHandler.hpp"
#include <sys/select.h>

CGIHandler::CGIHandler(const Response &response) 
    : _envp(nullptr), _cgiPath(""), _scriptName(""), _queryString(""), _bodyInput(""), _cgiOutput("") {
    // Initialize the CGI handler with the response
    // This constructor can be used to set up any initial state if needed
    std::unordered_map<std::string, std::string> envVariables = initEnvironmentVars(response.getRequest(), response.getLocationConfig()->getCgiPath());
    }

 CGIHandler::handle(const Request& req) {
    try {
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
        scriptPath = "myWebsite" + scriptPath;
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

std::unordered_map<std::string, std::string> CGIHandler::initEnvironmentVars(const Request& req, const std::string& scriptPath) {
    
    std::unordered_map<std::string, std::string> envVariables;
    // Initialize environment variables for CGI execution
    envVariables["REQUEST_METHOD"] = request.getMethod();
    envVariables["SCRIPT_NAME"] = _scriptName;
    envVariables["QUERY_STRING"] = _queryString;
    envVariables["CONTENT_TYPE"] = request.getHeaders().at("Content-Type");
    envVariables["CONTENT_LENGTH"] = std::to_string(request.getBody().size());
    envVariables["SERVER_NAME"] = request.getHeaders().at("Host");
    envVariables["SERVER_PROTOCOL"] = request.getHttpVersion();
    envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
    // Add more environment variables as needed
    return envVariables;
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

std::vector<char> CGIHandler::executeScript(const Request& req, const std::string& scriptPath) {
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

        std::string scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));
        if (chdir(scriptDir.c_str()) != 0) {
            perror("CGI chdir failed");
            exit(1);
        }

        std::string scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);

        //prepare environment
        std::vector<std::string> envStrings = buildEnvironmentStrings(req, scriptPath);
        char** envArray = buildEnvironmentArray(envStrings);

        std::string interpreter = getInterpreter(scriptPath);

        //execute script using execve
        if (!interpreter.empty()) {
            char* args[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptName.c_str()), nullptr};
            execve(interpreter.c_str(), args, envArray);
        } else {
            char* args[] = {const_cast<char*>(scriptName.c_str()), nullptr};
            execve(scriptName.c_str(), args, envArray);
        }
        //execve failed if we get here
        perror("CGI execve failed");
        exit(1);
    }

    //parent process
    //close unused pipes
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    //send request body to script stdin (for POST)
    if (req.getMethod() == "POST") {
        std::string body = req.getBody();
        if (!body.empty()) {
            write(stdin_pipe[1], body.c_str(), body.length());
        }
    }

    //close stdin to signal EOF
    close(stdin_pipe[1]);

    //read output from stdout_pipe[0] and stderr_pipe[0] using select()
    std::vector<char> output;
    std::vector<char> errorOutput;
    char buffer[4096];
    ssize_t bytesRead;

    fd_set read_fds;
    struct timeval timeout;
    int stdout_fd = stdout_pipe[0];
    int stderr_fd = stderr_pipe[0];

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

Response CGIHandler::parseOutput(const std::vector<char>& output) {
    Response response;

    response.setStatusCode(200);
    response.setReasonPhrase("OK");

    if (output.empty()) {
        response.setStatusCode(204);
        response.setReasonPhrase("No Content");
        return response;
    }

    //convert output to string for easier parsing
    std::string outputStr(output.begin(), output.end());

    //find separator between headers and body
    size_t headerEnd = outputStr.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
       //no headers, treat output as body
       response.setBody(output);
       response.addHeader("Content-Type", "text/html");
       response.addHeader("Content-Lenght", std::to_string(output.size()));
       return response;
    }

    //extract headers
    std::string headersStr = outputStr.substr(0, headerEnd);
    std::istringstream headerStream(headersStr);
    std::string line;

    //parse each header line
    while (std::getline(headerStream, line)) {
        if (line.empty() || line == "\r") continue;

        //remove trailing \r if present
        if (!line.empty() && line[line.length() -1] == '\r') {
            line.erase(line.length() - 1);
        }
        //find the colon separator
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            //trim whitespace
            value.erase(0, value.find_first_not_of(" \t"));

            //check for status header
            if (name == "Status") {
                size_t spacePos = value.find(' ');
                if (spacePos != std::string::npos) {
                    int statusCode = std::stoi(value.substr(0, spacePos));
                    std::string reasonPhrase = value.substr(spacePos + 1);
                    response.setStatusCode(statusCode);
                    response.setReasonPhrase(reasonPhrase);
                }
            } else {
                response.addHeader(name, value);
            }
        }
    }

    //extract body
    std::vector<char> body(output.begin() + headerEnd + 4, output.end());
    response.setBody(body);

    //if content type not set, default to text/html
    if (response.getHeaders().find("Content-Type") == response.getHeaders().end()) {
        response.addHeader("Content-Type", "text/html");
    }

    //set content length header if not set
    if (response.getHeaders().find("Content-Length") == response.getHeaders().end()) {
        response.addHeader("Content-Length", std::to_string(response.getBody().size()));
    }

    return response;
}