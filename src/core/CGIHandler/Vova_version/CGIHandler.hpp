#include "../Webserv.hpp"
#include "../response/Response.hpp"

class CGIHandler {
    private:
        std::unordered_map<std::string, std::string> _envVariables;
        char **_envp; // Environment variables for CGI execution
        std::string _cgiPath; // Path to the CGI script
        std::string _scriptName; // Name of the CGI script
        std::string _queryString; // Query string for the CGI script
        std::string _bodyInput; // Body input for the CGI script, if applicable
        std::string _cgiOutput; // Output from the CGI script execution
        int _stdinPipe[2];
        int _stdoutPipe[2];
        int _exitStatus;
        bool _timedOut;
        pid_t _childPid; // PID of the child process executing the CGI script

        const Request* _request;
        const LocationConfig* _location;

        void initEnvVars(const Request &request, const LocationConfig &location); // Initialize environment variables for CGI execution
        char **convertMapToEnvp(const std::unordered_map<std::string, std::string> &envMap) const; // Convert map to char** for execve

    public:
        CGIHandler(const Response &response); // Constructor to initialize from Response object
        CGIHandler(const CGIHandler &src);
        CGIHandler &operator=(const CGIHandler &src);
        ~CGIHandler();

        void setCgiPath(const std::string &cgiPath);
        void setScriptName(const std::string &scriptName);
        void setQueryString(const std::string &queryString);
        void addEnvVariable(const std::string &key, const std::string &value);
        std::string getCgiPath() const;
        std::string getScriptName() const;
        std::string getQueryString() const;
        std::unordered_map<std::string, std::string> getEnvVariables() const;
        std::string generateCgiCommand() const; // Generate the command to execute the CGI script
        std::string executeCgiScript(); // Execute the CGI script and return the output
        void setDefaults(); // Set default values for CGI handler
        bool isValidCgiPath() const; // Check if the CGI path is valid


};