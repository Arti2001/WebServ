#include "CGIHandler.hpp"

CGIHandler::CGIHandler(const Response &response) : _cgiPath(""), _scriptName(""), _queryString("") {
  const Request *request = response.getRequest();
  const LocationConfig *location = response.getLocationConfig();

  _cgiPath = location->getCgiExtension().at(".py"); // Assuming .py is the CGI extension
  _scriptName = request->getPath();
  _queryString = request->getQuery();
  initEnvVars(*request, *location);
  _envp = convertMapToEnvp(_envVariables);
}

CGIHandler::CGIHandler(const CGIHandler &src)
    : _cgiPath(src._cgiPath), _scriptName(src._scriptName), _queryString(src._queryString), _envVariables(src._envVariables) {
    setDefaults();
}

CGIHandler &CGIHandler::operator=(const CGIHandler &src) {
    if (this != &src) {
        _cgiPath = src._cgiPath;
        _scriptName = src._scriptName;
        _queryString = src._queryString;
        _envVariables = src._envVariables;
    }
    return *this;
}

CGIHandler::~CGIHandler() {
    for (char **p = _envp; *p; ++p) {
        free(*p);
    }
    free(_envp);
}

void CGIHandler::initEnvVars(const Request &request, const LocationConfig &location) {
    _envVariables["REQUEST_METHOD"] = request.getMethod();
    _envVariables["SCRIPT_NAME"] = _scriptName;
    _envVariables["QUERY_STRING"] = _queryString;
    _envVariables["CONTENT_TYPE"] = request.getHeaders().at("Content-Type");
    _envVariables["CONTENT_LENGTH"] = std::to_string(request.getBody().size());
    _envVariables["SERVER_NAME"] = request.getHeaders().at("Host");
    _envVariables["SERVER_PROTOCOL"] = request.getHttpVersion();
    _envVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
    // Add more environment variables as needed

}

char **CGIHandler::convertMapToEnvp(const std::unordered_map<std::string, std::string> &envMap) const {
    char **envp = static_cast<char **>(malloc((envMap.size() + 1) * sizeof(char *)));
    if (!envp) {
        throw std::runtime_error("Failed to allocate memory for environment variables");
    }

    size_t i = 0;
    for (const auto &pair : envMap) {
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

std::string CGIHandler::generateCgiCommand() const {
    return _cgiPath + " " + _scriptName + "?" + _queryString;
}

std::string CGIHandler::executeCgiScript()  {

    if (pipe(_stdinPipe) == -1 || pipe(_stdoutPipe) == -1) {
        throw std::runtime_error("Pipe creation failed");
    }

    _childPid = fork();
    if (_childPid < 0) {
        throw std::runtime_error("Fork failed");
    } else if (_childPid == 0) {
        // Child process
        dup2(_stdinPipe[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
        dup2(_stdoutPipe[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        close(_stdinPipe[1]); // Close the write end of the stdin pipe
        close(_stdoutPipe[0]); // Close the read end of the stdout pipe
        char *argv[] = {const_cast<char *>(_cgiPath.c_str()), nullptr};
        execve(_cgiPath.c_str(), argv, _envp); // Execute the CGI script
        perror("execve failed");
        exit(EXIT_FAILURE); // Exit if execve fails
    }
    close(_stdinPipe[0]); // Close the read end of the stdin pipe in the parent
    close(_stdoutPipe[1]); // Close the write end of the stdout pipe in the parent
    if (!_bodyInput.empty()) {
        if (write(_stdinPipe[1], _bodyInput.c_str(), _bodyInput.size()) == -1)
            throw std::runtime_error("Failed to write to CGI stdin pipe");
    }
    close(_stdinPipe[1]); // Close the write end of the stdin pipe after writing
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(_stdoutPipe[0], buffer, sizeof(buffer))) > 0) {
        _cgiOutput.append(buffer, bytesRead); // Append the output to the CGI output string
    }
    close(_stdoutPipe[0]); // Close the read end of the stdout pipe
    int status;
    if (waitpid(_childPid, &status, 0) == -1) 
        throw std::runtime_error("waitpid failed"); 
    if (WIFEXITED(status)) {
        _exitStatus = WEXITSTATUS(status);
    } else {
        _exitStatus = -1; // Indicate that the CGI script did not exit normally
    }
    for (size_t i = 0; _envp[i]; i++) {
        delete[] _envp[i] ; // Free each environment variable string
    }
    delete[] _envp; // Free the environment variable array
    return _cgiOutput; // Return the CGI script output
}