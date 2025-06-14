#include "Client.hpp" 
// #include "Request/RequestParser.hpp"


Client::Client(int serverFd, ServerManager* serverManager) : _headersParsed(false), _clientBytesSent(0),
	_serverFd(serverFd), _serverManager(serverManager){
	}



Client::~Client() {
}

//Setters
void	Client::setLastActiveTime( std::time_t timeStamp) {
	this->_lastActiveTime = timeStamp;
}

void Client::setIsClosed(bool flag) {
	this->_closed = flag;
}




//Getters

std::time_t	Client::getLastActiveTime( void ) const {

	return(_lastActiveTime);
}

int	Client::getServerFd(void) const {
	return(_serverFd);
}

size_t& Client::getClientsBytesSent(void) {
	return(_clientBytesSent);
}

const std::string&	Client::getClientsResponse(void) const {
	return(_clientResponse);
}

//bool&	Client::getIsClosed(void) {
//	return(_closed);
//}




std::string	Client::getCgiResponse(Request &request) {

	CGIHandler	cgiHandler;
	std::cout << "we call CGI" << std::endl;
	Response cgiResponse = cgiHandler.handle(request);
	std::vector<char> respVector = cgiResponse.serialize();
	std::string respStr(respVector.begin(), respVector.end());

	return(respStr);
}

bool Client::isErrorCode(int statusCode)
{
	if (statusCode >= 400 && statusCode <= 599)
		return true;
	return false;
}


std::string	Client::getResponse(Request &request) {

	int									socketClientConnectedTo = this->getServerFd();
	int		_currentStatusCode = _request.getStatusCode();
	const std::vector<const vServer*>&	subServConfigs = _serverManager->findServerConfigsByFd(socketClientConnectedTo);
	if (isErrorCode(_currentStatusCode) || subServConfigs.empty()) {
		std::cerr << "No server configurations found for the connected socket." << std::endl;
		// Handle the error, e.g., return a 500 Internal Server Error response
		_currentStatusCode = 500;
	}
	std::string							hostHeaderValue = getAnyHeader(request.getHeaders(), "Host");
	if (isErrorCode(_currentStatusCode) || hostHeaderValue.empty()) {
		std::cerr << "Host header is missing in the request." << std::endl;
		// Handle the error, e.g., return a 400 Bad Request response
		_currentStatusCode = 400;
	}
	const vServer&						askedServConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);
	const Location&						askedLocationBlock = _serverManager->findLocationBlockByUri(askedServConfig, request.getUri());

	_response = Response(_request, askedLocationBlock, socketClientConnectedTo);
	_response.generateResponse();
	return (_response);
}



//curl -v -H "Host: server3.com" http://127.0.0.1:8055/

bool Client::headersComplete(const std::string& request) {
	// Check if the request ends with a double CRLF, indicating the end of headers
	size_t pos = request.find("\r\n\r\n");
	if (pos == std::string::npos) {
		return false; // No end of headers found
	}
	_bodyStart = pos + 4; // Set the start of the body after the headers
	return true;
}

bool Client::bodyComplete(const std::string& body) const {
	// Check if the body is complete based on the Content-Length header or chunked transfer encoding
	auto it = _request.getHeaders().find("Content-Length");
	if (it != _request.getHeaders().end()) {
		int contentLength = std::stoi(it->second);
		return body.size() >= static_cast<size_t>(contentLength);
	}
	if (_request.getIsChunked()) {
		auto it = body.find("\r\n0\r\n\r\n");
		if (it != std::string::npos) {
			// If we find the end of the chunked body, we consider it complete
			return true;
		}
		return false; // Chunked transfer encoding, but we haven't found the end yet
	}
	return false; // No Content-Length or chunked transfer encoding, so we can't determine completeness
}


void    Client::handleRequest (int clientFd) {
    char        requestBuffer[REQUEST_READ_BUFFER];//8KB
    ssize_t     bytesRead;
    
    bytesRead = recv(clientFd, requestBuffer, REQUEST_READ_BUFFER, 0);
    if (bytesRead > 0)
    {
		std::string incomingData(requestBuffer, bytesRead);
		std::cout << "Received data: " << incomingData << std::endl;
		if (!_headersParsed) {
			_startLineAndHeadersBuffer += incomingData;
			if (!headersComplete(_startLineAndHeadersBuffer)) {
				std::cout << "Request is still incomplete, waiting for more data..." << std::endl;
				return;
			}
			_request = Request(_startLineAndHeadersBuffer);
			try {
				_request.parseRequest();
			} catch(const std::exception& e) {
				std::cerr << "Failed to parse request: "<< e.what() << '\n';
				_serverManager->closeClientFd(clientFd);
				return;
			}
			_headersParsed = true;
			_startLineAndHeadersBuffer.clear(); // Clear the buffer after parsing headers
		}
		// If headers are parsed, we can now check for the body. It is optional depending on request type, 
		// so it is separated from the headers parsing logic.
		if (_headersParsed && _request.getBodyExpected())
		{
			if (_bodyBuffer.empty()) {
				// Capture the initial body segment from the combined header+body buffer
				_bodyBuffer = _startLineAndHeadersBuffer.substr(_bodyStart) + incomingData;
			} else {
				// Append subsequent chunks directly
				_bodyBuffer += incomingData;
			}
			if (bodyComplete(_bodyBuffer)) {
				_request.setBody(_bodyBuffer);
				_request.parseBody();
				_bodyBuffer.clear(); // Clear the body buffer after parsing
			}
		}
        return ;
    }
    else if (bytesRead == 0){
        std::cout << "Client disconnected: Clean up!" << std::endl;
        _serverManager->closeClientFd(clientFd);
        return;
    }
    else
        std::cout << "Recv failed"<< std::endl;
		_serverManager->closeClientFd(clientFd);
		return;
}


void	Client::handleResponse(int clientFd) {
	
	// here i first should do the following:
	// 1. have a function that will use the request to match the server and location block
	// 2. generate the response based on the request and location block. if it is cgi request
	// 3. send the response to the client
	
	size_t&		totalBytesSent = this->getClientsBytesSent();
	const char*	servResp = this->getClientsResponse().c_str();
	size_t		responseSize = this->getClientsResponse.size();
	ssize_t		bytesSent = send(clientFd, servResp + totalBytesSent, responseSize - totalBytesSent, 0);

	if (bytesSent == -1) {
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}
	totalBytesSent += bytesSent;
	if (totalBytesSent == responseSize) {
		std::cout << "All data sent" << "\n";
		_serverManager->closeClientFd(clientFd);
		totalBytesSent = 0;
	}
}

std::string Client::getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName) {

	std::unordered_map<std::string, std::string>::iterator it = headers.find(headerName);

	if (it != headers.end()) {
		return (it->second);
	}
	else {
		return ("");
	}
}
