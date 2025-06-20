/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:05:00 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/20 10:50:16 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() {}

Response::Response(Request *request, ServerManager *ServerManager, int clientSocket) : 
    _request(request),
    _serverManager(ServerManager),
    _serverConfig(nullptr),
    _locationConfig(nullptr),
    _clientSocket(clientSocket),
    _rawResponse("")
{
    _statusCode = request->getStatusCode();
    _statusMessage = _statusMessages[_statusCode];
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
    const std::vector<const vServer*>&	subServConfigs = _serverManager->findServerConfigsByFd(_clientSocket);
	if (subServConfigs.empty()) {
		std::cerr << "No server configurations found for the connected socket." << std::endl;
		// Handle the error, e.g., return a 500 Internal Server Error response
		setStatusCode(500);
        return ;
	}
    std::cout << "About to check host" << std::endl;
    std::string hostHeaderValue = Client::getAnyHeader(_request->getHeaders(), "Host");
    if (hostHeaderValue.empty()) {
        setStatusCode(400);
        return ;
    }
    std::cout << "Host found:" << hostHeaderValue << std::endl;
    
    _serverConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);
    std::cout << "Server config found: " << (_serverConfig ? "Yes" : "No") << std::endl;
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
    }
}

const std::string& Response::getRawResponse() const {
    return _rawResponse;
}

void Response::generateResponse() {
    if (_statusCode >= 400 && _statusCode < 600)
    return generateErrorResponse();

    if (_statusCode >= 300 && _statusCode < 400)
        return handleRedirectRequest();

    if (isCgiRequest())
        return handleCGIRequest();

    std::string method = _request->getMethod(); // Assuming this returns uppercase: "GET", "POST", etc.
    std::transform(method.begin(), method.end(), method.begin(), ::toupper); // Ensure method is uppercase
    if (method == "GET")
        handleGetRequest();
    else if (method == "POST")
        handlePostRequest();
    else if (method == "DELETE")
        handleDeleteRequest();
    else
    {
        std::cerr << "Unsupported method: " << method << std::endl;
        _statusCode = 405; // Method Not Allowed
        return generateErrorResponse(); // Method Not Allowed
    }
    createStartLine();
    createHeaders();
    createBody();
}

bool Response::isCgiRequest() const {
    // std::string path = _request->getPath(); // e.g., "/cgi-bin/script.cgi"
    // if (path.find(DEFAULT_CGI_DIRECTORY) != std::string::npos) {
    //     for (const auto & ext : _locationConfig->getCgiExtensions()) {
    //         if (path.find(ext) != std::string::npos) {
    //             return true; // The request is a CGI request
    //         }
    //     }
    // }
    return false; // The request is not a CGI request
}

std::string Response::resolveRelativePath(const std::string &path, const std::string &locationPath) const {
    // Resolve relative path based on location path
    if (path.empty() || path[0] == '/') {
        return path; // Absolute path, return as is
    }
    return locationPath + "/" + path; // Relative path, prepend location path
}

void Response::handleGetRequest() {
    if (!isMethodAllowed("GET")) {
        setStatusCode(405); // Method Not Allowed
        return generateErrorResponse();
    }
    std::string path = _request->getPath(); // e.g., "/images/cat.png"
    std::string fullPath = _locationConfig->getLocationRoot() + resolveRelativePath(path, _locationConfig->getLocationPath());

    if (!fileExists(fullPath)) {
        setStatusCode(404);
        return generateErrorResponse();
    }

    if (isLargeFile(fullPath) && _statusCode < 400) {
        makeChunkedResponse(fullPath);
    } else if (_statusCode >= 400 ) {
        generateErrorResponse();
    } else {
        makeRegularResponse(fullPath);
    }
}

void Response::generateErrorResponse() {
    std::cout << "Generating error response for status code: " << _statusCode << std::endl;
    setStatusMessage(_statusMessages[_statusCode]);
    
    // Clear existing headers and body
    _headers.clear();
    _body.clear();
    
    // Set default headers for error responses
    addHeader("Content-Type", "text/html");
    addHeader("Connection", "close");
    
    // Generate a simple HTML body for the error response
    _body = "<html><body><h1>" + std::to_string(_statusCode) + " " + _statusMessage + "</h1></body></html>";
    
    // Generate the full response
    createStartLine();
    createHeaders();
    createBody();
}

bool Response::isMethodAllowed(const std::string &method) const {
    std::vector<std::string> allowedMethods = _locationConfig->getLocationAllowedMethods();
    return std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
};


bool Response::fileExists(const std::string &path) {
    // need to confirm if file exists
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        return false; // File does not exist or cannot be accessed
    }
    return S_ISREG(fileStat.st_mode); // Check if it's a regular file
};

bool Response::isLargeFile(const std::string &path) {
    struct stat fileStat;
    // Use stat() to get file information
    if (stat(path.c_str(), &fileStat) == -1) {
        setStatusCode(404);
        return false;    // Could not access file
    }
    // Compare file size to buffer size
    return fileStat.st_size > LARGE_FILE_SIZE_THRESHOLD;
}

void Response::makeRegularResponse(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        setStatusCode(404);
        return generateErrorResponse();
    }
    std::ostringstream oss;
    oss << file.rdbuf(); // Read the entire file into a string
    _body = oss.str(); // Set the body of the response to the file content
    file.close();
    setStatusCode(200); // Set status code to 200 OK
    addHeader("Content-Length", std::to_string(_body.size())); // Add Content-Length header
    addHeader("Content-Type", getMimeType(path)); // Set content type to binary
};

std::string Response::getMimeType(const std::string &path) const {
    // Determine the MIME type based on the file extension
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
    // Default to application/octet-stream for unknown types
    return "application/octet-stream";
}

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
        _rawResponse += "0\r\n\r\n"; // End of chunked response
    }
    else
    {
        std::string chunkSize = std::to_string(bytesRead) + "\r\n";
        _rawResponse += chunkSize; // Add chunk size to the response
        _rawResponse.append(buffer, bytesRead); // Append the read bytes to the response
        _rawResponse += "\r\n"; // End of chunk    
    }
    addHeader("Transfer-Encoding", "chunked");
    addHeader("Content-Type", getMimeType(path)); // Set content type based on file extension
    setStatusCode(200); // Set status code to 200 OK
};

void Response::handleCGIRequest() {
    // Handle CGI request logic here
    
}

void Response::handleRedirectRequest() {
    // Handle redirect request logic here
    // This is a placeholder for the actual redirect handling logic
    if (_locationConfig) {
        std::pair<int, std::string> redirect = _locationConfig->getLocationReturnPages();
        if (redirect.first != 0) {
            setStatusCode(redirect.first);
            setStatusMessage(_statusMessages[redirect.first]);
            addHeader("Location", redirect.second);
            _body.clear(); // Clear body for redirect responses
            createStartLine();
            createHeaders();
        } else {
            setStatusCode(404); // If no redirect is set, return 404
            generateErrorResponse();
        }
    } else {
        setStatusCode(404); // No location config found, return 404
        generateErrorResponse();
    }
}

std::string Response::createUploadFile() {
    const std::string &body = _request->getBody();
    if (body.empty()) {
        setStatusCode(400); // Bad Request
        return "";
    }
    const std::string &uploadDirectory = _locationConfig->getLocationUploadPath();
    if (uploadDirectory.empty()) {
        setStatusCode(403); // Forbidden
        return "";
    }
    std::string fileName = generateUUID(); // Generate a unique file name
    std::string filePath;
    // Ensure the upload directory ends with a slash
    if (!uploadDirectory.empty() && uploadDirectory.back() != '/')
        filePath = uploadDirectory + "/" + fileName;
    else
        filePath = uploadDirectory + fileName;
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        setStatusCode(500); // Internal Server Error
        return "";
    }
    outFile.write(body.data(), body.size());
    outFile.close();
    return fileName; // Return the path of the uploaded file
}

std::string Response::generateUUID() {
    uuid_t uuid;
    char uuidStr[37]; // UUIDs are 36 characters plus the null terminator
    uuid_generate(uuid);
    uuid_unparse(uuid, uuidStr);
    return std::string(uuidStr);
}

void Response::handlePostRequest() {
    if (!isMethodAllowed("POST")) {
        setStatusCode(405); // Method Not Allowed
        return generateErrorResponse();
    }

    std::string fileName = createUploadFile();
    if (fileName.empty()) {
        return generateErrorResponse();
    }
    _body = "POST request handled successfully"; // Example response body
    _body += "\nFile ID: " + fileName; // Append the file ID to the response body
    setStatusCode(200); // Set status code to 200 OK
    addHeader("Content-Length", std::to_string(_body.size()));
    addHeader("Content-Type", "text/plain"); // Set content type to text/plain
}

void Response::handleDeleteRequest() {
    if (!isMethodAllowed("DELETE")) {
        setStatusCode(405); // Method Not Allowed
        return generateErrorResponse();
    }
    std::string path = _request->getPath(); // e.g., "/files/cat.png"
    // Prevent directory traversal
    if (path.find("..") != std::string::npos) {
        setStatusCode(400); // Bad Request
        return generateErrorResponse();
    }
    if (!path.empty() && path[0] == '/') // remove leading slash
        path = path.substr(1);
    std::string fullPath;
    if (_locationConfig->getLocationUploadPath().back() != '/')
        fullPath = _locationConfig->getLocationUploadPath() + "/" + path;
    else
        fullPath = _locationConfig->getLocationUploadPath() + path;
    if (!fileExists(fullPath)) {
        setStatusCode(404); // Not Found
        return generateErrorResponse();
    }
    
    if (remove(fullPath.c_str()) != 0) {
        setStatusCode(500); // Internal Server Error
        return generateErrorResponse();
    }
    _body = "DELETE request handled successfully"; // Example response body
    setStatusCode(200); // Set status code to 200 OK
    addHeader("Content-Type", "text/plain");
    addHeader("Content-Length", std::to_string(_body.size()));
}

void Response::createStartLine() {
    if (_statusMessages.find(_statusCode) == _statusMessages.end()) 
        _statusCode = 418; // Default to "I'm a teapot" if status code is not recognized 
    _statusMessage = _statusMessages[_statusCode];
    std::string startLine = _request->getHttpVersion() + " " + std::to_string(_statusCode) + " " + _statusMessage + "\r\n";
    _rawResponse += startLine;
}


void Response::createHeaders(){
    for (const auto &header : _headers) {
        _rawResponse += header.first + ": " + header.second + "\r\n";
    }
    _rawResponse += "\r\n"; // End of headers
    // need to add default headers as well as headers depending on request
}

std::string Response::generateDirectoryListing(const std::string& fsPath, const std::string& urlPath) {
    std::ostringstream html;
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1><ul>";

    DIR* dir = opendir(fsPath.c_str());
    if (!dir) {
        setStatusCode(500); // Internal Server Error
        return "";
    }

    struct dirent* entry;
    std::vector<std::string> entries;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        entries.push_back(name);
    }
    std::sort(entries.begin(), entries.end()); // Sort entries alphabetically
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
            encoded << c; // Keep alphanumeric and some special characters
        } else {
            encoded << '%' << std::uppercase << std::hex << static_cast<int>(c);
        }
    }
    return encoded.str();
}

void Response::createBody() {
    if (_body.empty()) {
        _rawResponse += "\r\n"; // No body, just end the response
    } else {
        _rawResponse += _body + "\r\n"; // Add body to the response
    }
}