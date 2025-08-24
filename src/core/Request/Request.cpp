/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:42 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 22:49:32 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : _currentPosition(0), _bodySize(REQUEST_DEFAULT_MAX_BODY_SIZE), _timeout(REQUEST_DEFAULT_TIMEOUT), _statusCode(REQUEST_DEFAULT_STATUS_CODE), _isChunked(false) {
}

Request::Request(const std::string &rawRequest) : _rawRequest(rawRequest), _currentPosition(0), _method(""), _path(""), _httpVersion(""), _query(""), _body(""), _bodySize(REQUEST_DEFAULT_MAX_BODY_SIZE), _timeout(REQUEST_DEFAULT_TIMEOUT), _statusCode(REQUEST_DEFAULT_STATUS_CODE), _isChunked(false), _isCgi(false), _bodyExpected(false), _headers(), _supportedMethods(), _seenHeaders() {
    registerSupportedMethods();
    if (_rawRequest.empty()) {
        std::cerr << "Empty request received." << std::endl;
        this->setStatusCode(400);
        return;
    }
}

void Request::registerSupportedMethods(void){
	_supportedMethods.insert("GET");
	_supportedMethods.insert("POST");
	_supportedMethods.insert("DELETE");
}

Request::Request(const Request &src) : _rawRequest(src._rawRequest), _currentPosition(src._currentPosition), _method(src._method), _path(src._path), _httpVersion(src._httpVersion), _query(src._query), _body(src._body), _bodySize(src._bodySize), _timeout(src._timeout), _statusCode(src._statusCode), _isChunked(src._isChunked), _isCgi(src._isCgi), _bodyExpected(src._bodyExpected), _headers(src._headers), _supportedMethods(src._supportedMethods), _seenHeaders(src._seenHeaders) {
}

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

Request::~Request() {
}

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

void Request::setTimeout(time_t timeout) {
    _timeout = timeout;
}

const std::string &Request::getMethod() const {
    return _method;
}

const std::string &Request::getPath() const {
    return _path;
}

const std::string &Request::getHttpVersion() const {
    return _httpVersion;
}

const std::unordered_map<std::string, std::string> &Request::getHeaders() const {
    return _headers;
}

const std::string &Request::getBody() const {
    return _body;
}

const std::string &Request::getQuery() const {
    return _query;
}

time_t Request::getTimeout() const {
    return _timeout;
}

int Request::getStatusCode() const {
    return _statusCode;
}

bool Request::getCgiStatus() const {
    return _isCgi;
}

void Request::setCgi(bool isCgi) {
    _isCgi = isCgi;
}

bool Request::checkError() const {
    if (_statusCode >= 400 && _statusCode < 600) {
        return true;
    }
    return false;
}

void Request::setStatusCode(int statusCode) {
    _statusCode = statusCode;
}

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