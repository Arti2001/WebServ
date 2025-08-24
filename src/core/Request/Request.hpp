/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:38 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:38:25 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <ctime>
#include <sstream>
#include <vector>
#include "../Utils.hpp"

#define REQUEST_DEFAULT_TIMEOUT 10 // Default timeout for the request in seconds
#define REQUEST_DEFAULT_STATUS_CODE 200 // Default status code for the request
#define REQUEST_DEFAULT_MAX_BODY_SIZE 1024 * 1024 * 1024 // Maximum body size for the request in bytes (1 GB)

/**
 * @brief HTTP request parser and handler class for the webserv application
 * @details This class is responsible for parsing raw HTTP requests, extracting method, URI, headers, 
 *          and body content. It supports various HTTP methods (GET, POST, DELETE), handles chunked 
 *          transfer encoding, and validates request components according to HTTP standards.
 * @note The class automatically validates HTTP methods, versions, and body sizes, setting appropriate 
 *       status codes for malformed requests.
 */
class Request {
    public:
        // Constructors and Destructor
        /**
         * @brief Default constructor for Request object
         * @return None
         * @note Creates a default Request with empty fields and default timeout/status values
         */
        Request();

        /**
         * @brief Constructor with raw request string
         * @param rawRequest The raw HTTP request string to parse
         * @note Automatically registers supported methods and validates the request
         */
        Request(const std::string &rawRequest);

        /**
         * @brief Copy constructor
         * @param src The Request object to copy from
         * @note Performs deep copy of all member variables
         */
        Request(const Request &src);

        /**
         * @brief Assignment operator
         * @param src The Request object to assign from
         * @return Reference to this Request object
         * @note Performs self-assignment check to avoid unnecessary operations
         */
        Request &operator=(const Request &src);

        /**
         * @brief Destructor
         * @return None
         * @note Automatically cleans up allocated resources
         */
        ~Request();

        // Core parsing methods
        /**
         * @brief Parses the complete HTTP request including start line, headers, and body
         * @return None
         * @note Stops parsing if errors are encountered during any step
         */
        void parseRequest();

        /**
         * @brief Parses the first line of the HTTP request containing method, URI, and HTTP version
         * @return None
         * @note Advances _currentPosition to skip the newline character after parsing
         */
        void parseStartLine();

        /**
         * @brief Parses and validates the HTTP method from the request
         * @param method The HTTP method string to parse
         * @return None
         * @note Sets status code to 400 for empty methods or 405 for unsupported methods
         */
        void parseMethod(const std::string &method);

        /**
         * @brief Parses the request URI, separating the path from query parameters
         * @param uri The URI string to parse
         * @return None
         * @note Sets status code to 400 if URI is empty
         */
        void parseUri(const std::string &uri);

        /**
         * @brief Parses and validates the HTTP version from the request
         * @param httpVersion The HTTP version string to parse
         * @return None
         * @note Sets status code to 400 for empty versions or 505 for unsupported versions
         */
        void parseHttpVersion(const std::string &httpVersion);

        /**
         * @brief Parses all HTTP headers from the request
         * @return None
         * @note Sets status code to 400 for invalid headers or missing headers
         */
        void parseHeaders();

        /**
         * @brief Parses the request body, handling both regular and chunked transfer encoding
         * @return None
         * @note Sets status code to 413 if body size exceeds limits
         */
        void parseBody();

        /**
         * @brief Parses a chunked transfer-encoded request body
         * @return None
         * @note Sets status code to 400 for invalid chunk sizes
         */
        void parseChunkedBody();

        // Configuration methods
        /**
         * @brief Sets the timeout value for the request
         * @param timeout The timeout value in seconds
         * @return None
         */
        void setTimeout(time_t timeout);

        /**
         * @brief Sets the status code for the request
         * @param statusCode The HTTP status code to set
         * @return None
         */
        void setStatusCode(int statusCode);

        /**
         * @brief Sets the CGI flag for the request
         * @param isCgi Boolean value indicating if this is a CGI request
         * @return None
         */
        void setCgi(bool isCgi);

        /**
         * @brief Sets the request body
         * @param body The body content to set
         * @return None
         */
        void setBody(const std::string &body) { _body = body; }

        /**
         * @brief Resets all Request object fields to their default values
         * @return None
         * @note Don't clear _supportedMethods as it's constant
         */
        void reset(void);

        // Getter methods
        /**
         * @brief Returns the HTTP method of the request
         * @return const reference to the HTTP method string
         */
        const std::string &getMethod() const;

        /**
         * @brief Returns the request path
         * @return const reference to the request path string
         */
        const std::string &getPath() const;

        /**
         * @brief Returns the HTTP version of the request
         * @return const reference to the HTTP version string
         */
        const std::string &getHttpVersion() const;

        /**
         * @brief Returns the request headers as an unordered map
         * @return const reference to the headers map
         */
        const std::unordered_map<std::string, std::string> &getHeaders() const;

        /**
         * @brief Returns the request body
         * @return const reference to the request body string
         */
        const std::string &getBody() const;

        /**
         * @brief Returns the size of the request body
         * @return The body size in bytes
         */
        unsigned int getBodySize() const { return _bodySize; }

        /**
         * @brief Returns the query string from the URI
         * @return const reference to the query string
         */
        const std::string &getQuery() const;

        /**
         * @brief Returns the timeout value for the request
         * @return The timeout value in seconds
         */
        time_t getTimeout() const;

        /**
         * @brief Returns the current status code of the request
         * @return The HTTP status code
         */
        int getStatusCode() const;

        /**
         * @brief Returns whether the request is a CGI request
         * @return true if CGI request, false otherwise
         */
        bool getCgiStatus() const;

        /**
         * @brief Returns whether the body is expected in the request
         * @return true if body is expected, false otherwise
         */
        bool getBodyExpected() const { return _bodyExpected; }

        /**
         * @brief Returns whether the request body is chunked
         * @return true if chunked, false otherwise
         */
        bool getIsChunked() const { return _isChunked; }

        /**
         * @brief Returns the request URI
         * @return const reference to the request URI string
         */
        const std::string &getUri() const { return _path; }

        // Utility methods
        /**
         * @brief Prints the request details for debugging purposes
         * @return None
         * @note Outputs method, path, HTTP version, headers, body, and query to stdout
         */
        void printRequest() const;

        /**
         * @brief Checks if the request has an error status code
         * @return true if status code indicates an error (4xx or 5xx), false otherwise
         */
        bool checkError() const;

        /**
         * @brief Converts a hexadecimal string to an integer
         * @param hex The hexadecimal string to convert
         * @return The integer value, or -1 if conversion fails
         * @note Uses std::stoi with base 16 for conversion
         */
        int hexToInt(const std::string &hex) const;

    private:
        // Helper methods
        /**
         * @brief Registers the supported HTTP methods (GET, POST, DELETE)
         * @return None
         * @note Called during construction to populate the supported methods set
         */
        void registerSupportedMethods();

        /**
         * @brief Checks if the request should have a body based on Content-Length and Transfer-Encoding headers
         * @return true if body is expected, false otherwise
         * @note Validates Content-Length against limits and checks Transfer-Encoding validity
         */
        bool checkBodyRelatedHeaders();

        // Request data attributes
        std::string _rawRequest; // Raw HTTP request string
        size_t _currentPosition; // Current position in the request stream
        std::string _method; // HTTP method (GET, POST, DELETE)
        std::string _path; // Request path
        std::string _httpVersion; // HTTP version (HTTP/1.1, HTTP/2.0)
        std::string _query; // Query string from the URL
        std::string _body; // Request body
        unsigned _bodySize; // Size of the request body

        // Configuration attributes
        time_t _timeout; // Timeout for the request
        int _statusCode; // Status code for the request (e.g., 200, 404, 500)
        bool _isChunked; // Flag to indicate if the request body is chunked
        bool _isCgi; // Flag to indicate if the request is a CGI request
        bool _bodyExpected; // Flag to indicate if the body is expected in the request

        // Headers and methods
        std::unordered_map<std::string, std::string> _headers; // Request headers
        std::unordered_set<std::string> _supportedMethods; // Set of supported HTTP methods
        std::unordered_set<std::string> _seenHeaders; // Set of seen headers to avoid duplicates
};

#endif