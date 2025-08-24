/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:05:00 by pminialg      #+#    #+#                 */
/*   Updated: 2025/08/24 21:42:27 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() {}

Response::Response(Request *request, ServerManager *ServerManager, int serverFd, int clientFd) : 
    _request(request),
    _serverManager(ServerManager),
    _serverConfig(nullptr),
    _locationConfig(nullptr),
    _serverFd(serverFd),
	_clientFd(clientFd),
	_isCgi(false),
	_cgiHandler(nullptr),
	_statusCode(200),
	_validPath(false),
    _rawResponse(""),
	_statusMessage("OK"),
	_headers(),
	_body(""),
	_statusMessages({
		{200, "OK"},
		{301, "Moved Permanently"},
		{400, "Bad Request"},
		{401, "Unauthorized"},
		{403, "Forbidden"},
		{404, "Not Found"},
		{405, "Method Not Allowed"},
		{408, "Request Timeout"},
		{413, "Payload Too Large"},
		{418, "I'm a teapot"},
		{429, "Too Many Requests"},
		{500, "Internal Server Error"}
	}),
	_cgiIndexFile("")
{
    _statusCode = request->getStatusCode();
    _statusMessage = _statusMessages[_statusCode];
    _validPath = false;
    matchServer();
    matchLocation();
}

Response::~Response() {}

int Response::getStatusCode() const {
    return _statusCode;
}

void Response::setStatusCode(int statusCode) {
    _statusCode = statusCode;
}

const std::string& Response::getStatusMessage() const {
    return _statusMessage;
}

void Response::setStatusMessage(const std::string& statusMessage) {
    _statusMessage = statusMessage;
}

void Response::addHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

const std::unordered_map<std::string, std::string>& Response::getHeaders() const {
    return _headers;
}

void Response::setBody(const std::string &body) {
    _body = body;
}

const std::string& Response::getBody() const {
    return _body;
}

void Response::matchServer() {
    const std::vector<const vServer*>&	subServConfigs = _serverManager->findServerConfigsByFd(_serverFd);
	if (subServConfigs.empty()) {
		std::cerr << "No server configurations found for the connected socket." << std::endl;
		setStatusCode(500);
        return ;
	}
    std::string hostHeaderValue = Client::getAnyHeader(_request->getHeaders(), "Host");
    if (hostHeaderValue.empty()) {
        setStatusCode(400);
        return ;
    }
    
    _serverConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);
    if (!_serverConfig) {
        setStatusCode(404);
        return ;
    }
}

void Response::matchLocation() {
    if (!_serverConfig)
    {
        std::cerr << "No server config found" << std::endl;
        setStatusCode(400);
        return;
    }
    _locationConfig = _serverManager->findLocationBlockByUri(*_serverConfig, _request->getUri());
    if (!_locationConfig) {
        std::cerr << "No matching location block found for the request URI. No default." << std::endl;
        setStatusCode(404);
		return;
    }
    if (_locationConfig->getLocationReturnPages().first) {
        setStatusCode(_locationConfig->getLocationReturnPages().first);
	}
    if (_locationConfig->getLocationClientMaxSize() < _request->getBodySize())
        setStatusCode(413);
}

const std::string& Response::getRawResponse() const {
    return _rawResponse;
}

void Response::generateResponse() {
    if (_statusCode >= 400 && _statusCode < 600)
    {
        std::cout << "Error response with " << _statusCode << " after matching." << std::endl;
        return generateErrorResponse();
    }

    if (_statusCode >= 300 && _statusCode < 400)
        return handleRedirectRequest();
    if (isCgiRequest())
        return handleCGIRequest();

    std::string method = _request->getMethod();
    std::transform(method.begin(), method.end(), method.begin(), ::toupper);
    if (method == "GET")
        handleGetRequest();
    else if (method == "POST")
        handlePostRequest();
    else if (method == "DELETE")
        handleDeleteRequest();
    else
    {
        std::cerr << "Unsupported method: " << method << std::endl;
        setStatusCode(405);
        return generateErrorResponse();
    }
    if (_statusCode >= 400) {
        return;
    }
    createStartLine();
    createHeaders();
    createBody();
}

void Response::generateErrorResponse() {
    std::cout << "Generating error response for status code: " << _statusCode << std::endl;
    setStatusMessage(_statusMessages[_statusCode]);
    
    _headers.clear();
    _body.clear();
    
	std::cout << "Checking for custom error page for status code: " << _statusCode << std::endl;
	if (_locationConfig && _locationConfig->getLocationErrorPages().find(_statusCode) != _locationConfig->getLocationErrorPages().end()) {
		std::cout << "Custom error page found for status code: " << _statusCode << std::endl;
		std::string errorPagePath = _locationConfig->getLocationErrorPages().at(_statusCode);
        std::string fullPath = _locationConfig->getLocationRoot() + resolveRelativePath(errorPagePath, _locationConfig->getLocationPath());
        std::cout << "Full path to error page: " << fullPath << std::endl;
		if (fileExists(fullPath)) {
			std::ifstream file(fullPath);
			std::ostringstream oss;
    		oss << file.rdbuf();
    		_body = oss.str();
    		file.close();
		}
	}
	else {
    _body = "<html><body><h1>" + std::to_string(_statusCode) + " " + _statusMessage + "</h1></body></html>";
	}
    addHeader("Content-Type", "text/html");
    addHeader("Connection", "close");
    
	_rawResponse.clear();
    createStartLine();
    createHeaders();
    createBody();
}

void Response::handleRedirectRequest() {
    if (_locationConfig) {
        std::pair<int, std::string> redirect = _locationConfig->getLocationReturnPages();
        if (redirect.first != 0) {
            setStatusCode(redirect.first);
            setStatusMessage(_statusMessages[redirect.first]);
            addHeader("Location", redirect.second);
            _body.clear();
            createStartLine();
            createHeaders();
        } else {
            setStatusCode(404);
            generateErrorResponse();
        }
    } else {
        setStatusCode(404);
        generateErrorResponse();
    }
}

bool Response::isCgiRequest() {
    std::string path = _request->getUri();
    size_t extDot = path.find_last_of('.');
    std::string extension;
    if (extDot != std::string::npos)
        extension = path.substr(path.find_last_of('.'));
    else
        extension = "";
    std::map <std::string, std::string> cgiExtensions = _locationConfig->getLocationAllowedCgi();
    if (cgiExtensions.empty()) {
        return false;
    }
     if (extension.empty()) {
        std::vector<std::string> indexFiles = _locationConfig->getLocationIndex();
        if (indexFiles.empty()) {
            std::cerr << "No index files specified for this location." << std::endl;
            return false;
        }
        for (const auto &indexFile : indexFiles) {
            extDot = indexFile.find_last_of('.');
            std::string indexExtension;
            if (extDot != std::string::npos)
                indexExtension = indexFile.substr(extDot);
            else
                indexExtension = "";
            if (cgiExtensions.find(indexExtension) != cgiExtensions.end() && fileExists(indexFile)) {
				_cgiIndexFile = indexFile;
                return true;
            }
        }
    }
    else if (cgiExtensions.find(extension) != cgiExtensions.end()) {
        return true;
    } 
   return false;
}

void Response::handleCGIRequest() {
    if (!isMethodAllowed(_request->getMethod())) {
        setStatusCode(405);
        return generateErrorResponse();
    }
    try {
        _cgiHandler = std::make_unique<CGIHandler>(*_request, *_locationConfig, _cgiIndexFile);
        _cgiHandler->start();
		_isCgi = true;
		_serverManager->addCgiFdToMap(_cgiHandler->getStdoutFd(), _clientFd);
		_serverManager->addCgiFdToMap(_cgiHandler->getStderrFd(), _clientFd);
    }
    catch (const CGIHandler::CGIException &e) {
        std::cerr << "CGI Exception: " << e.what() << std::endl;
        setStatusCode(e.statusCode());
        return generateErrorResponse();
    }
}

void Response::generateCGIResponse(){
	_rawResponse.clear();
	_rawResponse = _cgiHandler->finalize();
	if (_rawResponse.empty()) {
		setStatusCode(500);
		return generateErrorResponse();
	}
}

void Response::handleGetRequest() {
    if (!isMethodAllowed("GET")) {
        setStatusCode(405);
        return generateErrorResponse();
    }
    std::string path = _request->getPath();
    std::string fullPath = _locationConfig->getLocationRoot() + resolveRelativePath(path, _locationConfig->getLocationPath());
    
    if (!fileExists(fullPath) && !_validPath) {
        setStatusCode(404);
        return generateErrorResponse();
    }
    else if (_validPath && !fileExists(fullPath))
    {
		std::string indexPath;
        bool foundIndex = false;
		for (const std::string& locIndex : _locationConfig->getLocationIndex()) {
			indexPath = fullPath + "/" + locIndex;
			if (fileExists(indexPath))
			{
				fullPath = indexPath;
                foundIndex = true;
                break;
            }
		}
		if (!foundIndex) {
			if (_locationConfig->getLocationAutoIndex()) {
				_body = generateDirectoryListing(fullPath, _locationConfig->getLocationPath());
				return ;
			}
			else {
				setStatusCode(404);
				return generateErrorResponse();
			}
		}
	}

    if (isLargeFile(fullPath) && _statusCode < 400) {
        makeChunkedResponse(fullPath);
    } else if (_statusCode >= 400 ) {
        generateErrorResponse();
    } else {
        makeRegularResponse(fullPath);
    }
}

void Response::makeRegularResponse(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        setStatusCode(404);
        return generateErrorResponse();
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    _body = oss.str();
    file.close();
    setStatusCode(200);
    addHeader("Content-Length", std::to_string(_body.size()));
    addHeader("Content-Type", getMimeType(path));
};

void Response::makeChunkedResponse(const std:: string &path) {
    char buffer[RESPONSE_READ_BUFFER_SIZE];
    int file = open(path.c_str(), O_RDONLY);
    ssize_t bytesRead = read(file, buffer, RESPONSE_READ_BUFFER_SIZE);
    if (bytesRead == -1)
    {
        setStatusCode(404);
        return generateErrorResponse();
    }
    else if (bytesRead == 0)
    {
        close(file);
        _rawResponse += "0\r\n\r\n";
    }
    else
    {
        std::string chunkSize = intToHex(bytesRead) + "\r\n";
        _rawResponse += chunkSize;
        _rawResponse.append(buffer, bytesRead);
        _rawResponse += "\r\n";
    }
    addHeader("Transfer-Encoding", "chunked");
    addHeader("Content-Type", getMimeType(path));
    setStatusCode(200);
};

std::string Response::intToHex(int value) {
	std::ostringstream oss;
	oss << std::hex << value;
	return oss.str();
}

std::string Response::createUploadFile() {
    const std::string &body = _request->getBody();
    if (body.empty()) {
        setStatusCode(400);
        return "";
    }
    const std::string &uploadDirectory = _locationConfig->getLocationUploadPath();
    if (uploadDirectory.empty()) {
        setStatusCode(403);
        return "";
    }
    std::string fileName = generateUUID();
    std::string filePath;
    if (!uploadDirectory.empty() && uploadDirectory.back() != '/')
        filePath = uploadDirectory + "/" + fileName;
    else
        filePath = uploadDirectory + fileName;
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        setStatusCode(500);
        return "";
    }
    outFile.write(body.data(), body.size());
    outFile.close();
    return fileName;
}

std::string Response::generateUUID() {
    uuid_t uuid;
    char uuidStr[37];
    uuid_generate(uuid);
    uuid_unparse(uuid, uuidStr);
    return std::string(uuidStr);
}

void Response::handlePostRequest() {
    if (!isMethodAllowed("POST")) {
        setStatusCode(405);
        return generateErrorResponse();
    }

    std::string fileName = createUploadFile();
    if (fileName.empty()) {
        std::cout << "Error creating upload file" << std::endl;
        return generateErrorResponse();
    }
    _body = "POST request handled successfully";
    _body += "\nFile ID: " + fileName;
    setStatusCode(200);
    addHeader("Content-Length", std::to_string(_body.size()));
    addHeader("Content-Type", "text/plain");
}

void Response::handleDeleteRequest() {
    if (!isMethodAllowed("DELETE")) {
        setStatusCode(405);
        return generateErrorResponse();
    }
    std::string path = _request->getPath();
    if (path.find("..") != std::string::npos) {
        setStatusCode(400);
        return generateErrorResponse();
    }
    if (!path.empty() && path[0] == '/')
        path = path.substr(1);
    std::string fullPath;
    if (_locationConfig->getLocationUploadPath().back() != '/')
        fullPath = _locationConfig->getLocationUploadPath() + "/" + path;
    else
        fullPath = _locationConfig->getLocationUploadPath() + path;
    if (!fileExists(fullPath)) {
        std::cout << "File to delete not found" << std::endl;
        setStatusCode(404);
        return generateErrorResponse();
    }
    
    if (remove(fullPath.c_str()) != 0) {
        setStatusCode(500);
        return generateErrorResponse();
    }
    _body = "DELETE request handled successfully";
    setStatusCode(200);
    addHeader("Content-Type", "text/plain");
    addHeader("Content-Length", std::to_string(_body.size()));
}

void Response::createStartLine() {
    if (_statusMessages.find(_statusCode) == _statusMessages.end()) 
        setStatusCode(418);
    _statusMessage = _statusMessages[_statusCode];
    std::string startLine = _request->getHttpVersion() + " " + std::to_string(_statusCode) + " " + _statusMessage + "\r\n";
    _rawResponse += startLine;
}

void Response::createHeaders(){
    for (const auto &header : _headers) {
        _rawResponse += header.first + ": " + header.second + "\r\n";
    }
    _rawResponse += "\r\n";
}

std::string Response::generateDirectoryListing(const std::string& fsPath, const std::string& urlPath) {
    std::ostringstream html;
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1><ul>";

    DIR* dir = opendir(fsPath.c_str());
    if (!dir) {
        setStatusCode(500);
        return "";
    }

    struct dirent* entry;
    std::vector<std::string> entries;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        entries.push_back(name);
    }
    std::sort(entries.begin(), entries.end());
    for (const std::string& name : entries) {
        html << "<li><a href=\"" << urlEncode(name) << "\">" << name << "</a></li>";
    }
    closedir(dir);

    html << "</ul></body></html>";
    return html.str();
}

std::string Response::urlEncode(const std::string& value) {
    std::ostringstream encoded;
    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::uppercase << std::hex << static_cast<int>(c);
        }
    }
    return encoded.str();
}

void Response::createBody() {
    if (_body.empty()) {
        _rawResponse += "\r\n";
    } else {
        _rawResponse += _body + "\r\n";
    }
}

std::string Response::resolveRelativePath(const std::string &path, const std::string &locationPath) const {
	std::string new_path = path;
    if (new_path.empty()) {
        return path;
    }
	if (new_path[0] == '/') {
		return new_path.erase(0, 1);
	}
    return locationPath + "/" + path;
}

bool Response::isMethodAllowed(const std::string &method) const {
    const std::unordered_set<std::string>& allowedMethods = _locationConfig->getLocationAllowedMethods();
    return (allowedMethods.count(method));
}

bool Response::fileExists(const std::string &path) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        return false;
    }
    _validPath = true;
    return S_ISREG(fileStat.st_mode);
};

bool Response::isLargeFile(const std::string &path) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        setStatusCode(404);
        return false;
    }
    return fileStat.st_size > LARGE_FILE_SIZE_THRESHOLD;
}

std::string Response::getMimeType(const std::string &path)  {
    std::string extension = path.substr(path.find_last_of('.'));
    if (extension == ".html" || extension == ".htm") {
        return "text/html";
    } else if (extension == ".css") {
        return "text/css";
    } else if (extension == ".js") {
        return "application/javascript";
    } else if (extension == ".png") {
        return "image/png";
    } else if (extension == ".jpg" || extension == ".jpeg") {
        return "image/jpeg";
    } else if (extension == ".gif") {
        return "image/gif";
    } else if (extension == ".svg") {
        return "image/svg+xml";
    } else if (extension == ".txt") {
        return "text/plain";
    }
    return "application/octet-stream";
}