/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:05:00 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/17 16:18:09 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : {}

Response::Response(Request *request, ServerManager *ServerManager, int clientSocket) : 
    _request(request),
    _serverManager(ServerManager),
    _clientSocket(clientSocket)
{
    _statusCode = request->getStatusCode();
    _statusMessage = _statusMessages[_statusCode];
    matchServer();
    matchLocation();
}

Response::~Response() {}

int Response::getStatusCode() const {
    return _status_code;
}

void Response::setStatusCode(int statusCode) {
    _status_code = statusCode;
}

const std::string& Response::getStatusMessage() const {
    return _status_message;
}

void Response::setStatusMessage(const std::string& statusMessage) {
    _status_message = statusMessage;
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
const std::string Response::getBody() const {
    return _body;
}

void matchServer() {
    const std::vector<const vServer*>&	subServConfigs = _serverManager->findServerConfigsByFd(socketClientConnectedTo);
	if (subServConfigs.empty()) {
		std::cerr << "No server configurations found for the connected socket." << std::endl;
		// Handle the error, e.g., return a 500 Internal Server Error response
		_response.setStatusCode(500);
        return ;
	}
    std::string hostHeaderValue = _request->getHeaders().at("Host");
    if (hostHeaderValue.empty()) {
        _response.setStatusCode(400);
        return ;
    }
    _serverConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);
    if (!_serverConfig) {
        _response.setStatusCode(404);
        return ;
    }
}

void matchLocation() {
    const Location* defaultLocation = _serverManager->findDefaultLocationBlock(_serverConfig.getServerLocations());
    if (!defaultLocation) {
        std::cerr << "No default location block found for the server." << std::endl;
        // Handle the error, e.g., return a 500 Internal Server Error response
        _response.setStatusCode(500);
        return ;
    }
    _location = _serverManager->findLocationBlockByUri(_serverConfig, _request->getUri());
    if (!_location) {
        _location = defaultLocation;
        std::cerr << "No matching location block found for the request URI. Using default." << std::endl;
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

    if (_isCgiRequest())
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
    std::string path = _request->getPath(); // e.g., "/cgi-bin/script.cgi"
    if (path.find(DEFAULT_CGI_DIRECTORY) != std::string::npos) {
        for (const auto & ext : _locationConfig->getCgiExtensions()) {
            if (path.find(ext) != std::string::npos) {
                return true; // The request is a CGI request
            }
        }
    }
    return false; // The request is not a CGI request
}

void Response::handleGetRequest() {
    if (!isMethodAllowed("GET")) {
        setStatusCode(405); // Method Not Allowed
        return generateErrorResponse();
    }
    std::string path = _request->getPath(); // e.g., "/images/cat.png"
    std::string fullPath = _locationConfig->getRoot() + resolveRelativePath(path, _locationConfig->getPath());

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
    setStatusMessage(_statusMessages[_statusCode]);
    
    // Clear existing headers and body
    _headers.clear();
    _body.clear();
    
    // Set default headers for error responses
    addHeader("Content-Type", "text/html");
    
    // Generate a simple HTML body for the error response
    _body = "<html><body><h1>" + std::to_string(_statusCode) + " " + _statusMessage + "</h1></body></html>";
    
    // Generate the full response
    createStartLine();
    createHeaders();
    createBody();
}

bool Response::isMethodAllowed(const std::string &method) const {
    std::unordered_set<std::string> allowedMethods = _locationConfig->getAllowedMethods();
    auto it = allowedMethods.find(method);
    return it != allowedMethods.end();
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
        _response += "0\r\n\r\n"; // End of chunked response
    }
    else
    {
        std::string chunkSize = std::to_string(bytesRead) + "\r\n";
        _response += chunkSize; // Add chunk size to the response
        _response.append(buffer, bytesRead); // Append the read bytes to the response
        _response += "\r\n"; // End of chunk    
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
        std::pair<int, std::string> redirect = _locationConfig->getRedirect();
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
    const std::string &uploadDirectory = _locationConfig->getUploadDirectory();
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
    if (_locationConfig->getUploadDirectory().back() != '/')
        fullPath = _locationConfig->getUploadDirectory() + "/" + path;
    else
        fullPath = _locationConfig->getUploadDirectory() + path;
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
    if (_statusMessages.find(_statusCode) = = _statusMessages.end()) 
        _statusCode = 418; // Default to "I'm a teapot" if status code is not recognized 
    _statusMessage = _statusMessages[_statusCode];
    std::string startLine = _request->getHttpVersion() + " " + std::to_string(_statusCode) + " " + _statusMessage + "\r\n";
    _response += startLine;
}


void Response::createHeaders(){
    for (const auto &header : _headers) {
        _response += header.first + ": " + header.second + "\r\n";
    }
    _response += "\r\n"; // End of headers
    // need to add default headers as well as headers depending on request
}

std::string Response::generateDirectoryListing(const std::string& fsPath, const std::string& urlPath) {
    std::ostringstream html;
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1><ul>";

    DIR* dir = opendir(fsPath.c_str());
    if (!dir) {
        setStatusCode(500); // Internal Server Error
        return generateErrorResponse();
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
