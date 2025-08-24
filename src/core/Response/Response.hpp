/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:04:57 by pminialg      #+#    #+#                 */
/*   Updated: 2025/08/24 21:20:48 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../Client.hpp"
#include "../Request/Request.hpp"
#include "../ServerManager.hpp"
#include "../CGIHandler/CGIHandler.hpp"
#include "../parsingConfFile/vServer.hpp"
#include "../parsingConfFile/LocationConfig.hpp"
#include <uuid/uuid.h>
#include <sys/stat.h>
#include <dirent.h>

#define DEFAULT_CGI_DIRECTORY "/cgi-bin/"
#define LARGE_FILE_SIZE_THRESHOLD 1048576 // 1 MB
#define RESPONSE_READ_BUFFER_SIZE 8192 // Buffer size for reading response data

class ServerManager;
class CGIHandler;

/**
 * @brief HTTP response generator and handler class for the webserv application
 * @details This class is responsible for generating HTTP responses based on requests, handling 
 *          different HTTP methods (GET, POST, DELETE), managing CGI requests, serving static files, 
 *          and generating error responses. It supports both regular and chunked transfer encoding,
 *          directory listings, and custom error pages.
 * @note The class automatically matches server and location configurations, handles file uploads,
 *       and manages CGI script execution through the CGIHandler class.
 */
class Response {
    public:
        // Constructors and Destructor
        Response();
        Response(Request *request, ServerManager *serverManager, int serverFd, int clientfFd);
        Response(const Response &src);
        Response &operator=(const Response &src);
        ~Response();

        // Core response generation
        void generateResponse();
        void generateErrorResponse();
        void generateCGIResponse();

        // HTTP method handlers
        void handleGetRequest();
        void handlePostRequest();
        void handleDeleteRequest();
        void handleCGIRequest();
        void handleRedirectRequest();

        // Response building methods
        void createStartLine();
        void createHeaders();
        void createBody();
        void makeRegularResponse(const std::string &path);
        void makeChunkedResponse(const std::string &path);

        // Status line methods
        int getStatusCode() const;
        void setStatusCode(int statusCode);
        const std::string& getStatusMessage() const;
        void setStatusMessage(const std::string& reasonPhrase);

        // Header methods
        void addHeader(const std::string& key, const std::string& value);
        const std::unordered_map<std::string, std::string>& getHeaders() const;

        // Body methods
        void setBody(const std::string &body);
        const std::string &getBody() const;
        const std::string& getRawResponse() const;

        // Utility methods
        std::string urlEncode(const std::string& value);
        std::string intToHex(int value);
        static std::string getMimeType(const std::string &path);

        // Getters
        CGIHandler* getCgiHandler() const { return _cgiHandler.get(); }
        bool getIsCGI() const { return _isCgi; }
        int getClientFd() const { return _clientFd; }

    private:
        // Configuration and matching methods
        void matchServer();
        void matchLocation();
        bool isMethodAllowed(const std::string &method) const;
        bool isCgiRequest();

        // File handling methods
        bool fileExists(const std::string &path);
        bool isLargeFile(const std::string &path);
        std::string resolveRelativePath(const std::string &path, const std::string &locationPath) const;
        std::string generateDirectoryListing(const std::string& fsPath, const std::string& urlPath);

        // Upload and file creation methods
        std::string createUploadFile();
        std::string generateUUID();

        // Request and server attributes
        const Request *_request; // Pointer to the HTTP request object
        ServerManager *_serverManager; // Server manager to access server configurations
        const vServer *_serverConfig; // Virtual server configuration for the response
        const Location *_locationConfig; // Location configuration for the response
        int _serverFd; // Server file descriptor for the response
        int _clientFd; // Client file descriptor for the response

        // CGI handling attributes
        bool _isCgi; // Flag to indicate if this is a CGI response
        std::unique_ptr<CGIHandler> _cgiHandler; // CGI handler for processing CGI requests
        std::string _cgiIndexFile; // CGI index file for the location, if applicable

        // Response attributes
        int _statusCode; // HTTP status code (e.g., 200, 404, 500)
        std::string _statusMessage; // HTTP status message (e.g., "OK", "Not Found")
        std::string _rawResponse; // Complete HTTP response string
        std::string _body; // Body of the response
        std::unordered_map<std::string, std::string> _headers; // HTTP headers for the response

        // Path and file attributes
        bool _validPath; // Flag to indicate if the path is valid
        std::unordered_map<int, std::string> _statusMessages; // Map of status codes to messages
};

#endif