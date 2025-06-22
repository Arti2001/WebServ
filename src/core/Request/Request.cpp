#include "Request.hpp"

Request::Request() : _currentPosition(0), _timeout(REQUEST_DEFAULT_TIMEOUT), _statusCode(REQUEST_DEFAULT_STATUS_CODE), _bodySize(REQUEST_DEFAULT_MAX_BODY_SIZE), _isChunked(false) {
    // Default constructor
}

Request::Request(const std::string &rawRequest) : _currentPosition(0), _timeout(REQUEST_DEFAULT_TIMEOUT), _statusCode(REQUEST_DEFAULT_STATUS_CODE), _bodySize(REQUEST_DEFAULT_MAX_BODY_SIZE), _isChunked(false) {
    // Constructor with raw request string
    registerSupportedMethods();
    _rawRequest = rawRequest;
    if (_rawRequest.empty()) {
        std::cerr << "Empty request received." << std::endl;
        this->setStatusCode(400); // Bad Request
        return;
    }
}

void Request::registerSupportedMethods(void){
	_supportedMethods.insert("GET");
	_supportedMethods.insert("POST");
	_supportedMethods.insert("DELETE");
}

Request::Request(const Request &src) : _rawRequest(src._rawRequest), _currentPosition(src._currentPosition), _method(src._method), _path(src._path), _httpVersion(src._httpVersion), _headers(src._headers), _body(src._body), _query(src._query), _timeout(src._timeout), _statusCode(src._statusCode), _bodySize(src._bodySize), _isChunked(src._isChunked), _supportedMethods(src._supportedMethods) {
    // Copy constructor
}

Request &Request::operator=(const Request &src) {
    // Assignment operator
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
Request::~Request() {
    // Destructor
}

void Request::parseRequest() {
    // Parse start line
    parseStartLine();
    if (checkError()) return;
    // Parse headers
    parseHeaders();
    if (checkError()) return;
    // Check if body is expected
    _bodyExpected = checkBodyRelatedHeaders();
    if (_bodyExpected && _headers.find("Content-Length") != _headers.end()) {
        _bodySize = std::stoi(_headers["Content-Length"]);
    }  
}

bool Request::checkBodyRelatedHeaders() {
    auto it = _headers.find("Content-Length");
    auto it2 = _headers.find("Transfer-Encoding");
    if (it == _headers.end() && it2 == _headers.end())
        return false;
    if (it != _headers.end() && !it->second.empty() && std::stoi(it->second) > _bodySize) {
        std::cerr << "Request body size exceeds the limit." << std::endl;
        this->setStatusCode(413); // Payload Too Large
        return false;
    }
    if (it != _headers.end() && it2 != _headers.end() && it2->second != "chunked") {
        std::cerr << "Invalid Transfer-Encoding header: " << it2->second << std::endl;
        this->setStatusCode(400); // Bad Request
        return false; // Invalid request
    }
    if (_headers["Transfer-Encoding"] == "chunked")
        _isChunked = true; // If Transfer-Encoding is chunked, set the flag
    return true; // Body size headers are valid
}

void Request::parseStartLine() {
    // Parse the start line of the request (method, path(request target), HTTP version)
    std::string startLine;
    while (_currentPosition < _rawRequest.size() && _rawRequest[_currentPosition] != '\n') {
        startLine += _rawRequest[_currentPosition++];
    }
    std::cout << "Start line: " << startLine << std::endl;
    std::istringstream ss(startLine);
    std::string method, uri, httpVersion;
    ss >> method >> uri >> httpVersion;
    std::cout << "Parsing start line: " << method << " " << uri << " " << httpVersion << std::endl;
    parseMethod(method);
    parseUri(uri);
    parseHttpVersion(httpVersion);
    _currentPosition++; // Skip the newline character after the start line
}

void Request::parseMethod(const std::string &method) {
    // Parse the HTTP method from the request
    if (method.empty()) {
        std::cerr << "Empty method in request." << std::endl;
        return this->setStatusCode(400); // Bad Request
    }
    if (_supportedMethods.find(method) == _supportedMethods.end()) {
        std::cerr << "Unsupported method: " << method << std::endl;
        return this->setStatusCode(405); // Method Not Allowed
    }
    _method = method;
}

void Request::parseHttpVersion(const std::string &httpVersion) {
    // Parse the HTTP version from the request
    if (httpVersion.empty()) {
        std::cerr << "Empty HTTP version in request." << std::endl;
        return this->setStatusCode(400); // Bad Request
    }
    if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/2.0") {
        std::cerr << "Unsupported HTTP version: " << httpVersion << std::endl;
        return this->setStatusCode(505); // HTTP Version Not Supported
    }
    _httpVersion = httpVersion;
}

void Request::parseUri(const std::string &uri) {
    // Parse the request path
    if (uri.empty()) {
        std::cerr << "Empty URI in request." << std::endl;
        return this->setStatusCode(400); // Bad Request
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

void Request::parseHeaders() {
    // Parse the headers from the request
    if (_currentPosition >= _rawRequest.size()) {
        std::cerr << "No headers found in request." << std::endl;
        return this->setStatusCode(400); // Bad Request
    }
    std::istringstream ss(&_rawRequest[_currentPosition]);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty() || line == "\r") {
            break; // End of headers
        }
       
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string headerName = line.substr(0, pos);
            std::string headerValue = line.substr(pos + 1);
            Utils::trim(headerValue);
            if (headerName.empty() || headerValue.empty()) {
                std::cerr << "Invalid header: " << line << std::endl;
                return this->setStatusCode(400); // Bad Request, invalid header
            }
            std::cout << "Parsed header:" << headerName << ": " << headerValue << std::endl;
            _headers[headerName] = headerValue;
        } else {
            std::cerr << "Invalid header format: " << line << std::endl;
            return this->setStatusCode(400); // Bad Request, invalid header format
        }
        _currentPosition += line.size() + 1; // Move past the header line and newline character
    }
}

void Request::parseBody() {
    // Parse the body of the request
    if (!_bodyExpected) {
        std::cerr << "No body for this request." << std::endl;
        _body = ""; // Set body to empty if no body is present
        return; // No body to parse
    }
    if (_body.size() > static_cast<unsigned long>(_bodySize)) {
        std::cerr << "Request body size exceeds the limit imposed by header." << std::endl;
        this->setStatusCode(413); // Payload Too Large
        return; // Exit if body size exceeds the limit
    }
    if (_isChunked)
        parseChunkedBody();
}

void Request::parseChunkedBody() {
    // Parse the chunked body of the request
    std::istringstream ss(_body);
    std::string chunk;
    int chunkSize = 0;
    _body.clear(); // Clear the body before parsing chunks
    while (std::getline(ss, chunk)) {
        if (chunk.empty()) {
            continue; // Skip empty lines
        }
        size_t pos = chunk.find("\r\n");
        if (pos != std::string::npos) {
            chunk = chunk.substr(0, pos); // Remove the trailing CRLF
        }        
        try {
            chunkSize = std::stoi(chunk, nullptr, 16); // Convert hex size to integer
        }
        catch (const std::invalid_argument &e) {
            std::cerr << "Invalid chunk size: " << chunk << std::endl;
            this->setStatusCode(400); // Bad Request
            return; // Exit if chunk size is invalid
        }
        if (chunkSize <= 0) {
            break; // End of chunks
        }
        _body += ss.str().substr(ss.tellg(), chunkSize); // Append the chunk to the body
        ss.seekg(chunkSize + 2, std::ios::cur); // Move past the chunk and CRLF
    }
    // std::string chunked_data = _body;
    // std::string decoded_data;
    // size_t pos = 0;

    // while (pos < chunked_data.length()) {
    //     size_t line_end = chunked_data.find("\r\n", pos);
    //     if (line_end == std::string::npos) {
    //         // incomplete chunk
    //         break;
    //     }

    //     //extract and convert the chunk size
    //     std::string chunk_size_hex = chunked_data.substr(pos, line_end - pos);
    //     size_t chunk_size = 0;
    //     std::istringstream(chunk_size_hex) >> std::hex >> chunk_size;

    //     // if chunk size == 0, we've reached the end of the body
    //     if (chunk_size == 0) {
    //         _body = decoded_data;
    //         return ;
    //     }

    //     // position after the CRLF
    //     pos = line_end + 2;

    //     // check if we have enough data for this chunk
    //     if (pos + chunk_size + 2 > chunked_data.length()) {
    //         // not enough data
    //         break;
    //     }

    //     decoded_data.append(chunked_data.substr(pos, chunk_size));

    //     //move position past this chunk
    //     pos += chunk_size + 2;
    // }
    // throw std::runtime_error("400 Bad Request: Incomplete chunked transfer encoding");
}

void Request::setTimeout(time_t timeout) {
    // Set the timeout for the request
    _timeout = timeout;
}

const std::string &Request::getMethod() const {
    // Get the HTTP method
    return _method;
}
const std::string &Request::getPath() const {
    // Get the request path
    return _path;
}
const std::string &Request::getHttpVersion() const {
    // Get the HTTP version
    return _httpVersion;
}
const std::unordered_map<std::string, std::string> &Request::getHeaders() const {
    // Get the request headers
    return _headers;
}
const std::string &Request::getBody() const {
    // Get the request body
    return _body;
}
const std::string &Request::getQuery() const {
    // Get the query string
    return _query;
}

time_t Request::getTimeout() const {
    // Get the timeout for the request
    return _timeout;
}

int Request::getStatusCode() const {
    // Get the status code for the request
    return _statusCode;
}

bool Request::getCgiStatus() const {
    // Check if the request is a CGI request
    return _isCgi;
}

void Request::setCgi(bool isCgi) {
    // Set the CGI flag for the request
    _isCgi = isCgi;
}

bool Request::checkError() const {
    // Check if there is an error in the request
    std::cout << "Checking for errors in the request..." << _statusCode << std::endl;
    if (_statusCode >= 400 && _statusCode < 600) {
        return true; // Error status code
    }
    return false; // No error
}

void Request::setStatusCode(int statusCode) {
    // Set the status code for the request
    _statusCode = statusCode;
}

void Request::printRequest() const {
    // Print the request for debugging purposes
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

int Request::hexToInt(const std::string &hex) const {
    // Convert a hexadecimal string to an integer
    int value = 0;
    try {
        value = std::stoi(hex, nullptr, 16);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid hexadecimal string: " << hex << std::endl;
        return -1; // Return -1 for invalid hex
    }
    return value;
}