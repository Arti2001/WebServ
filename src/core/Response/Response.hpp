/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:04:57 by pminialg      #+#    #+#                 */
/*   Updated: 2025/08/24 21:39:14 by vovashko      ########   odam.nl         */
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
        /**
         * @brief Default constructor for Response object
         * @return None
         * @note Creates an empty Response with default values
         */
        Response();

        /**
         * @brief Constructor with request and server information
         * @param request Pointer to the HTTP request object
         * @param serverManager Pointer to the server manager
         * @param serverFd Server file descriptor
         * @param clientfFd Client file descriptor
         * @note Automatically matches server and location configurations
         */
        Response(Request *request, ServerManager *serverManager, int serverFd, int clientfFd);

        /**
         * @brief Copy constructor
         * @param src The Response object to copy from
         * @note Performs deep copy of all member variables
         */
        Response(const Response &src);

        /**
         * @brief Assignment operator
         * @param src The Response object to assign from
         * @return Reference to this Response object
         * @note Performs self-assignment check to avoid unnecessary operations
         */
        Response &operator=(const Response &src);

        /**
         * @brief Destructor
         * @return None
         * @note Automatically cleans up allocated resources
         */
        ~Response();

        // Core response generation
        /**
         * @brief Generates the complete HTTP response based on the request method and status
         * @return None
         * @note Routes to appropriate handler based on status code and HTTP method
         */
        void generateResponse();

        /**
         * @brief Generates an error response with appropriate status code and message
         * @return None
         * @note Checks for custom error pages and falls back to default HTML error page
         */
        void generateErrorResponse();

        /**
         * @brief Generates the final CGI response from the CGI handler
         * @return None
         * @note Sets status code to 500 if CGI response is empty
         */
        void generateCGIResponse();

        // HTTP method handlers
        /**
         * @brief Handles GET requests by serving files or generating directory listings
         * @return None
         * @note Sets status code to 405 if method not allowed, 404 if file not found
         */
        void handleGetRequest();

        /**
         * @brief Handles POST requests by creating upload files
         * @return None
         * @note Sets status code to 405 if method not allowed, calls createUploadFile for file creation
         */
        void handlePostRequest();

        /**
         * @brief Handles DELETE requests by removing files
         * @return None
         * @note Sets status code to 405 if method not allowed, 400 for directory traversal attempts, 404 if file not found, 500 if deletion fails
         */
        void handleDeleteRequest();

        /**
         * @brief Handles CGI requests by creating a CGI handler and setting up file descriptors
         * @return None
         * @note Sets status code to 405 if method not allowed, 500 if CGI handler creation fails
         */
        void handleCGIRequest();

        /**
         * @brief Handles redirect requests based on location configuration
         * @return None
         * @note Sets Location header and generates appropriate redirect response
         */
        void handleRedirectRequest();

        // Response building methods
        /**
         * @brief Creates the HTTP start line with version, status code, and message
         * @return None
         * @note Sets status code to 418 if status code is not recognized
         */
        void createStartLine();

        /**
         * @brief Creates the HTTP headers section of the response
         * @return None
         * @note Adds all stored headers and ends with double CRLF
         */
        void createHeaders();

        /**
         * @brief Creates the response body section
         * @return None
         * @note Adds body content if present, or just CRLF if no body
         */
        void createBody();

        /**
         * @brief Creates a regular response by reading the entire file into memory
         * @param path The file path to read
         * @return None
         * @note Sets status code to 404 if file cannot be opened
         */
        void makeRegularResponse(const std::string &path);

        /**
         * @brief Creates a chunked response for large files
         * @param path The file path to read
         * @return None
         * @note Sets status code to 404 if file cannot be read
         */
        void makeChunkedResponse(const std::string &path);

        // Status line methods
        /**
         * @brief Gets the current status code of the response
         * @return The HTTP status code
         */
        int getStatusCode() const;

        /**
         * @brief Sets the status code for the response
         * @param statusCode The HTTP status code to set
         * @return None
         */
        void setStatusCode(int statusCode);

        /**
         * @brief Gets the status message for the current status code
         * @return const reference to the status message string
         */
        const std::string& getStatusMessage() const;

        /**
         * @brief Sets the status message for the response
         * @param statusMessage The status message to set
         * @return None
         */
        void setStatusMessage(const std::string& reasonPhrase);

        // Header methods
        /**
         * @brief Adds a header to the response
         * @param key The header name
         * @param value The header value
         * @return None
         */
        void addHeader(const std::string& key, const std::string& value);

        /**
         * @brief Gets all response headers
         * @return const reference to the headers map
         */
        const std::unordered_map<std::string, std::string>& getHeaders() const;

        // Body methods
        /**
         * @brief Sets the response body
         * @param body The body content to set
         * @return None
         */
        void setBody(const std::string &body);

        /**
         * @brief Gets the response body
         * @return const reference to the body string
         */
        const std::string &getBody() const;

        /**
         * @brief Gets the raw HTTP response string
         * @return const reference to the raw response string
         */
        const std::string& getRawResponse() const;

        // Utility methods
        /**
         * @brief URL-encodes a string for use in HTML links
         * @param value The string to encode
         * @return The URL-encoded string
         * @note Keeps alphanumeric characters and some special characters, encodes others as percent-encoded hex
         */
        std::string urlEncode(const std::string& value);

        /**
         * @brief Converts an integer to a hexadecimal string
         * @param value The integer value to convert
         * @return The hexadecimal string representation
         */
        std::string intToHex(int value);

        /**
         * @brief Determines the MIME type based on file extension
         * @param path The file path to analyze
         * @return The MIME type string
         * @note Defaults to application/octet-stream for unknown extensions
         */
        static std::string getMimeType(const std::string &path);

        // Getters
        /**
         * @brief Gets the CGI handler for this response
         * @return Pointer to the CGI handler, or nullptr if not a CGI response
         */
        CGIHandler* getCgiHandler() const { return _cgiHandler.get(); }

        /**
         * @brief Checks if this is a CGI response
         * @return true if CGI response, false otherwise
         */
        bool getIsCGI() const { return _isCgi; }

        /**
         * @brief Gets the client file descriptor
         * @return The client file descriptor
         */
        int getClientFd() const { return _clientFd; }

    private:
        // Configuration and matching methods
        /**
         * @brief Matches the server configuration based on the request
         * @return None
         * @note Sets status code to 500 if no server config found, 400 if no Host header, 404 if server not found
         */
        void matchServer();

        /**
         * @brief Matches the location configuration based on the request URI
         * @return None
         * @note Sets status code to 400 if no server config, 404 if no location found, 413 if body too large
         */
        void matchLocation();

        /**
         * @brief Checks if the given HTTP method is allowed for the current location
         * @param method The HTTP method to check
         * @return true if method is allowed, false otherwise
         */
        bool isMethodAllowed(const std::string &method) const;

        /**
         * @brief Checks if the request is a CGI request based on file extension or index files
         * @return true if CGI request, false otherwise
         * @note Checks both direct CGI files and CGI index files in directories
         */
        bool isCgiRequest();

        // File handling methods
        /**
         * @brief Checks if a file exists at the given path
         * @param path The file path to check
         * @return true if file exists and is a regular file, false otherwise
         * @note Sets _validPath to true if file exists
         */
        bool fileExists(const std::string &path);

        /**
         * @brief Checks if a file is considered large based on size threshold
         * @param path The file path to check
         * @return true if file size exceeds threshold, false otherwise
         * @note Sets status code to 404 if file cannot be accessed
         */
        bool isLargeFile(const std::string &path);

        /**
         * @brief Resolves relative paths based on location path
         * @param path The path to resolve
         * @param locationPath The location path to resolve against
         * @return The resolved absolute path
         * @note Handles both absolute and relative paths appropriately
         */
        std::string resolveRelativePath(const std::string &path, const std::string &locationPath) const;

        /**
         * @brief Generates an HTML directory listing for the given file system path
         * @param fsPath The file system path to list
         * @param urlPath The URL path for the listing
         * @return HTML string containing the directory listing
         * @note Sets status code to 500 if directory cannot be opened
         */
        std::string generateDirectoryListing(const std::string& fsPath, const std::string& urlPath);

        // Upload and file creation methods
        /**
         * @brief Creates an upload file from the request body
         * @return The filename of the created file, or empty string on failure
         * @note Sets status code to 400 if body is empty, 403 if no upload directory, 500 if file creation fails
         */
        std::string createUploadFile();

        /**
         * @brief Generates a unique UUID for file naming
         * @return A UUID string
         * @note Uses libuuid to generate system-level unique identifiers
         */
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