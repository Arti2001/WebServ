/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:38 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/07/06 13:08:39 by vshkonda      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

// handle here the parsing of the request

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
#define REQUEST_DEFAULT_MAX_BODY_SIZE 1024 * 1024 * 1024 // Maximum body size for the request in bytes (10 MB)

class Request
{
    private:
        std::string _rawRequest; // Raw request string
        size_t _currentPosition; // Current position in the request stream
        std::string _method; // HTTP method (GET, POST, DELETE)
        std::string _path; // Request path
        std::string _httpVersion; // HTTP version (HTTP/1.1, HTTP/2.0)
        std::unordered_map<std::string, std::string> _headers; // Request headers
        std::string _body; // Request body
        std::string _query; // Query string from the URL
        time_t _timeout; // Timeout for the request
        int _statusCode; // Status code for the request (e.g., 200, 404, 500)
        int _bodySize; // Size of the request body
        bool _isChunked; // Flag to indicate if the request body is chunked
        bool _isCgi; // Flag to indicate if the request is a CGI request
        bool _bodyExpected; // Flag to indicate if the body is expected in the request

        std::unordered_set<std::string> _supportedMethods; // Set of supported HTTP methods
        std::unordered_set<std::string> _seenHeaders; // Set of seen headers to avoid duplicates

        void registerSupportedMethods(); // Register supported HTTP methods
        bool checkBodyRelatedHeaders(); // Check the headers related to body size

    public:
        Request();
        Request(const std::string &rawRequest); // Constructor with raw request string
        Request(const Request &src);
        Request &operator=(const Request &src);
        ~Request();

        void parseRequest(); // Parse the raw request string
        void setTimeout(time_t timeout); // Set the timeout for the request
        void setStatusCode(int statusCode); // Set the status code for the request
        void setCgi(bool isCgi); // Set the CGI flag for the request
        void setBody(const std::string &body) { _body = body; } // Set the request body
		void reset(void);

        const std::string &getMethod() const; // Get the HTTP method
        const std::string &getPath() const; // Get the request path
        const std::string &getHttpVersion() const; // Get the HTTP version
        const std::unordered_map<std::string, std::string> &getHeaders() const; // Get the request headers
        const std::string &getBody() const; // Get the request body
        int getBodySize() const { return _bodySize; } // Get the size of the request body
        const std::string &getQuery() const; // Get the query string
        time_t getTimeout() const; // Get the timeout for the request
        int getStatusCode() const; // Get the status code for the request
        bool getCgiStatus() const; // Check if the request is a CGI request
        bool getBodyExpected() const { return _bodyExpected; } // Check if the body is expected in the request
        bool getIsChunked() const { return _isChunked; } // Check if the request body is chunked
        const std::string &getUri() const { return _path; } // Get the request URI


        void parseStartLine(); // Parse the start line of the request (method, path(request target), HTTP version)
        void parseMethod(const std::string &method); // Parse the HTTP method from the request
        void parseUri(const std::string &path); // Parse the request path
        void parseHttpVersion(const std::string &httpVersion); // Parse the HTTP version from the request
        void parseHeaders(); // Parse the headers from the request
        void parseBody(); // Parse the body of the request
        void parseChunkedBody(); // Parse the chunked body of the request

        void printRequest() const; // Print the request for debugging purposes
        bool checkError() const; // Check if there is an error in the request
        int hexToInt(const std::string &hex) const; // Convert a hexadecimal string to an integer
        // std::ostream& operator<<(std::ostream& os, const Request& req) {
        //     os << "Method: " << req._method << "\n";
        //     os << "URI: " << req._uri << "\n";
        //     os << "Version: " << req._version << "\n";
        //     os << "Headers:\n";
        //     for (const auto& header : req._headers) {
        //         os << "  " << header.first << ": " << header.second << "\n";
        //     }
        //     os << "Body: " << req._body << "\n";
        //     os << "Form Data:\n";
        //     for (const auto& header : req._form_data) {
        //         os << "  " << header.first << ": " << header.second << "\n";
        //     }
        //     os << "Files:\n";
        //     for (const auto& header : req._files) {
        //         os << "  " << header.first << ": " << header.second << "\n";
        //     }
        //     return os;
        // }
};

#endif