/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:38 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:20:27 by vovashko      ########   odam.nl         */
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
        Request();
        Request(const std::string &rawRequest);
        Request(const Request &src);
        Request &operator=(const Request &src);
        ~Request();

        // Core parsing methods
        void parseRequest();
        void parseStartLine();
        void parseMethod(const std::string &method);
        void parseUri(const std::string &path);
        void parseHttpVersion(const std::string &httpVersion);
        void parseHeaders();
        void parseBody();
        void parseChunkedBody();

        // Configuration methods
        void setTimeout(time_t timeout);
        void setStatusCode(int statusCode);
        void setCgi(bool isCgi);
        void setBody(const std::string &body) { _body = body; }
        void reset(void);

        // Getter methods
        const std::string &getMethod() const;
        const std::string &getPath() const;
        const std::string &getHttpVersion() const;
        const std::unordered_map<std::string, std::string> &getHeaders() const;
        const std::string &getBody() const;
        unsigned int getBodySize() const { return _bodySize; }
        const std::string &getQuery() const;
        time_t getTimeout() const;
        int getStatusCode() const;
        bool getCgiStatus() const;
        bool getBodyExpected() const { return _bodyExpected; }
        bool getIsChunked() const { return _isChunked; }
        const std::string &getUri() const { return _path; }

        // Utility methods
        void printRequest() const;
        bool checkError() const;
        int hexToInt(const std::string &hex) const;

    private:
        // Helper methods
        void registerSupportedMethods();
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