/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:42 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:03:27 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

/**
 * @brief Initializes a new Request object with default values
 * @return None
 * @note Creates a default Request with empty fields and default timeout/status values
 */
Request::Request() : _currentPosition(0), _timeout(REQUEST_DEFAULT_TIMEOUT), _statusCode(REQUEST_DEFAULT_STATUS_CODE), _bodySize(REQUEST_DEFAULT_MAX_BODY_SIZE), _isChunked(false) {
}

/**
 * @brief Initializes a new Request object and parses the raw HTTP request string
 * @param rawRequest The raw HTTP request string to parse
 * @note Automatically registers supported methods and validates the request
 */
Request::Request(const std::string &rawRequest) : _rawRequest(rawRequest), _currentPosition(0), _method(""), _path(""), _httpVersion(""), _headers() ,_body(""), _query(""), _timeout(REQUEST_DEFAULT_TIMEOUT), _statusCode(REQUEST_DEFAULT_STATUS_CODE), _bodySize(REQUEST_DEFAULT_MAX_BODY_SIZE), _isChunked(false), _isCgi(false), _bodyExpected(false), _supportedMethods(), _seenHeaders() {
    registerSupportedMethods();
    if (_rawRequest.empty()) {
        std::cerr << "Empty request received." << std::endl;
        this->setStatusCode(400);
        return;
    }
}

/**
 * @brief Registers the supported HTTP methods (GET, POST, DELETE)
 * @return None
 * @note Called during construction to populate the supported methods set
 */
void Request::registerSupportedMethods(void){
	_supportedMethods.insert("GET");
	_supportedMethods.insert("POST");
	_supportedMethods.insert("DELETE");
}

/**
 * @brief Creates a copy of an existing Request object
 * @param src The Request object to copy from
 * @return None
 * @note Performs deep copy of all member variables
 */
Request::Request(const Request &src) : _rawRequest(src._rawRequest), _currentPosition(src._currentPosition), _method(src._method), _path(src._path), _httpVersion(src._httpVersion), _headers(src._headers), _body(src._body), _query(src._query), _timeout(src._timeout), _statusCode(src._statusCode), _bodySize(src._bodySize), _isChunked(src._isChunked), _supportedMethods(src._supportedMethods) {
}

/**
 * @brief Assigns the values from another Request object
 * @param src The Request object to assign from
 * @return Reference to this Request object
 * @note Performs self-assignment check to avoid unnecessary operations
 */
Request &Request::operator=(const Request &src) {
    if (this != &src) {
        _rawRequest = src._rawRequest;
        _currentPosition = src._currentPosition;
        _method = src._method;
        _path = src._path;
        _httpVersion = src._httpVersion;
        _headers = src._headers;
        _body = src._body;
        _query = src._query;
        _timeout = src._timeout;
        _statusCode = src._statusCode;
        _bodySize = src._bodySize;
        _isChunked = src._isChunked;
        _supportedMethods = src._supportedMethods;
    }
    return *this;
}

/**
 * @brief Destroys the Request object
 * @return None
 * @note Automatically cleans up allocated resources
 */
Request::~Request() {
}

/**
 * @brief Resets all Request object fields to their default values
 * @return None
 * @note Don't clear _supportedMethods as it's constant
 */
void Request::reset() {
    _rawRequest.clear();
    _currentPosition = 0;
    _method.clear();
    _path.clear();
    _httpVersion.clear();
    _headers.clear();
    _body.clear();
    _query.clear();
    _timeout = REQUEST_DEFAULT_TIMEOUT;
    _statusCode = REQUEST_DEFAULT_STATUS_CODE;
    _bodySize = REQUEST_DEFAULT_MAX_BODY_SIZE;
    _isChunked = false;
}

/**
 * @brief Parses the complete HTTP request including start line, headers, and body
 * @return None
 * @note Stops parsing if errors are encountered during any step
 */
void Request::parseRequest() {
    parseStartLine();
    if (checkError()) return;
    parseHeaders();
    if (checkError()) return;
    _bodyExpected = checkBodyRelatedHeaders();
    if (_bodyExpected && _headers.find("Content-Length") != _headers.end()) {
        _bodySize = std::stoul(_headers["Content-Length"]);
    }  
	if (!_bodyExpected)
		_bodySize = 0;
}

/**
 * @brief Checks if the request should have a body based on Content-Length and Transfer-Encoding headers
 * @return true if body is expected, false otherwise
 * @note Validates Content-Length against limits and checks Transfer-Encoding validity
 */
bool Request::checkBodyRelatedHeaders() {
    auto it = _headers.find("Content-Length");
    auto it2 = _headers.find("Transfer-Encoding");
    if (it == _headers.end() && it2 == _headers.end())
        return false;
    if (it != _headers.end() && !it->second.empty() && std::stoul(it->second) > _bodySize) {
        std::cerr << "Request body size exceeds the limit." << std::endl;
        this->setStatusCode(413);
        return false;
    }
    if (it != _headers.end() && it2 != _headers.end() && it2->second != "chunked") {
        std::cerr << "Invalid Transfer-Encoding header: " << it2->second << std::endl;
        this->setStatusCode(400);
        return false;
    }
    if (_headers["Transfer-Encoding"] == "chunked")
        _isChunked = true;
    return true;
}

/**
 * @brief Parses the first line of the HTTP request containing method, URI, and HTTP version
 * @return None
 * @note Advances _currentPosition to skip the newline character after parsing
 */
void Request::parseStartLine() {
    std::string startLine;
    while (_currentPosition < _rawRequest.size() && _rawRequest[_currentPosition] != '\n') {
        startLine += _rawRequest[_currentPosition++];
    }
    std::istringstream ss(startLine);
    std::string method, uri, httpVersion;
    ss >> method >> uri >> httpVersion;
    parseMethod(method);
    parseUri(uri);
    parseHttpVersion(httpVersion);
    _currentPosition++;
}

/**
 * @brief Parses and validates the HTTP method from the request
 * @param method The HTTP method string to parse
 * @return None
 * @note Sets status code to 400 for empty methods or 405 for unsupported methods
 */
void Request::parseMethod(const std::string &method) {
    if (method.empty()) {
        std::cerr << "Empty method in request." << std::endl;
        setStatusCode(400);
		return ;
    }
    if (_supportedMethods.find(method) == _supportedMethods.end()) {
        std::cerr << "Unsupported method: " << method << std::endl;
        this->setStatusCode(405);
    }
    _method = method;
}

/**
 * @brief Parses and validates the HTTP version from the request
 * @param httpVersion The HTTP version string to parse
 * @return None
 * @note Sets status code to 400 for empty versions or 505 for unsupported versions
 */
void Request::parseHttpVersion(const std::string &httpVersion) {
    if (httpVersion.empty()) {
        std::cerr << "Empty HTTP version in request." << std::endl;
        return this->setStatusCode(400);
    }
    if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/2.0") {
        std::cerr << "Unsupported HTTP version: " << httpVersion << std::endl;
        return this->setStatusCode(505);
    }
    _httpVersion = httpVersion;
}

/**
 * @brief Parses the request URI, separating the path from query parameters
 * @param uri The URI string to parse
 * @return None
 * @note Sets status code to 400 if URI is empty
 */
void Request::parseUri(const std::string &uri) {
    if (uri.empty()) {
        std::cerr << "Empty URI in request." << std::endl;
        return this->setStatusCode(400);
    }
    std::string query;
    std::string path;
    if (uri.find('?') != std::string::npos) {
        size_t pos = uri.find('?');
        path = uri.substr(0, pos);
        query = uri.substr(pos + 1);
    } else {
        path = uri;
    }
    _path = path;
    _query = query;
}

/**
 * @brief Parses all HTTP headers from the request
 * @return None
 * @note Sets status code to 400 for invalid headers or missing headers
 */
void Request::parseHeaders() {
    if (_currentPosition >= _rawRequest.size()) {
        std::cerr << "No headers found in request." << std::endl;
        return this->setStatusCode(400);
    }
    std::istringstream ss(&_rawRequest[_currentPosition]);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty() || line == "\r") {
            break;
        }
       
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string headerName = line.substr(0, pos);
            std::string headerValue = line.substr(pos + 1);
            Utils::trim(headerValue);
            if (headerName.empty() || headerValue.empty()) {
                std::cerr << "Invalid header: " << line << std::endl;
                return this->setStatusCode(400);
            }
            _headers[headerName] = headerValue;
        } else {
            std::cerr << "Invalid header format: " << line << std::endl;
            return this->setStatusCode(400);
        }
        _currentPosition += line.size() + 1;
    }
}

/**
 * @brief Parses the request body, handling both regular and chunked transfer encoding
 * @return None
 * @note Sets status code to 413 if body size exceeds limits
 */
void Request::parseBody() {
    if (!_bodyExpected) {
        std::cerr << "No body for this request." << std::endl;
        _body = "";
        return;
    }
    if (_body.size() > static_cast<unsigned long>(_bodySize)) {
        std::cerr << "Request body size exceeds the limit imposed by header." << std::endl;
        this->setStatusCode(413);
        return;
    }
    if (_isChunked)
        parseChunkedBody();
}

/**
 * @brief Parses a chunked transfer-encoded request body
 * @return None
 * @note Sets status code to 400 for invalid chunk sizes
 */
void Request::parseChunkedBody() {
    std::istringstream ss(_body);
    std::string chunk;
    int chunkSize = 0;
    _body.clear();
    while (std::getline(ss, chunk)) {
        if (chunk.empty()) {
            continue;
        }
        size_t pos = chunk.find("\r\n");
        if (pos != std::string::npos) {
            chunk = chunk.substr(0, pos);
        }        
        try {
            chunkSize = std::stoi(chunk, nullptr, 16);
        }
        catch (const std::invalid_argument &e) {
            std::cerr << "Invalid chunk size: " << chunk << std::endl;
            this->setStatusCode(400);
            return;
        }
        if (chunkSize <= 0) {
            break;
        }
        _body += ss.str().substr(ss.tellg(), chunkSize);
        ss.seekg(chunkSize + 2, std::ios::cur);
    }
}

/**
 * @brief Sets the timeout value for the request
 * @param timeout The timeout value in seconds
 * @return None
 */
void Request::setTimeout(time_t timeout) {
    _timeout = timeout;
}

/**
 * @brief Returns the HTTP method of the request
 * @return const reference to the HTTP method string
 */
const std::string &Request::getMethod() const {
    return _method;
}

/**
 * @brief Returns the request path
 * @return const reference to the request path string
 */
const std::string &Request::getPath() const {
    return _path;
}

/**
 * @brief Returns the HTTP version of the request
 * @return const reference to the HTTP version string
 */
const std::string &Request::getHttpVersion() const {
    return _httpVersion;
}

/**
 * @brief Returns the request headers as an unordered map
 * @return const reference to the headers map
 */
const std::unordered_map<std::string, std::string> &Request::getHeaders() const {
    return _headers;
}

/**
 * @brief Returns the request body
 * @return const reference to the request body string
 */
const std::string &Request::getBody() const {
    return _body;
}

/**
 * @brief Returns the query string from the URI
 * @return const reference to the query string
 */
const std::string &Request::getQuery() const {
    return _query;
}

/**
 * @brief Returns the timeout value for the request
 * @return The timeout value in seconds
 */
time_t Request::getTimeout() const {
    return _timeout;
}

/**
 * @brief Returns the current status code of the request
 * @return The HTTP status code
 */
int Request::getStatusCode() const {
    return _statusCode;
}

/**
 * @brief Returns whether the request is a CGI request
 * @return true if CGI request, false otherwise
 */
bool Request::getCgiStatus() const {
    return _isCgi;
}

/**
 * @brief Sets the CGI flag for the request
 * @param isCgi Boolean value indicating if this is a CGI request
 * @return None
 */
void Request::setCgi(bool isCgi) {
    _isCgi = isCgi;
}

/**
 * @brief Checks if the request has an error status code
 * @return true if status code indicates an error (4xx or 5xx), false otherwise
 */
bool Request::checkError() const {
    if (_statusCode >= 400 && _statusCode < 600) {
        return true;
    }
    return false;
}

/**
 * @brief Sets the status code for the request
 * @param statusCode The HTTP status code to set
 * @return None
 */
void Request::setStatusCode(int statusCode) {
    _statusCode = statusCode;
}

/**
 * @brief Prints the request details for debugging purposes
 * @return None
 * @note Outputs method, path, HTTP version, headers, body, and query to stdout
 */
void Request::printRequest() const {
    std::cout << "Method: " << _method << std::endl;
    std::cout << "Path: " << _path << std::endl;
    std::cout << "HTTP Version: " << _httpVersion << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto &header : _headers) {
        std::cout << "   " <<header.first << ": " << header.second << std::endl;
    }
    std::cout << "Body: " << _body << std::endl;
    std::cout << "Query: " << _query << std::endl;
}

/**
 * @brief Converts a hexadecimal string to an integer
 * @param hex The hexadecimal string to convert
 * @return The integer value, or -1 if conversion fails
 * @note Uses std::stoi with base 16 for conversion
 */
int Request::hexToInt(const std::string &hex) const {
    int value = 0;
    try {
        value = std::stoi(hex, nullptr, 16);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid hexadecimal string: " << hex << std::endl;
        return -1;
    }
    return value;
}