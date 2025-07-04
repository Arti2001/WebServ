#include "Client.hpp" 
// #include "Request/RequestParser.hpp"


Client::Client(int serverFd, ServerManager* serverManager) : _headersParsed(false), _clientBytesSent(0),
	_serverFd(serverFd), _serverManager(serverManager), _closeAfterResponse(false){
	}



Client::~Client() {
}

Client::Client(const Client& other) : _request(other._request), _startLineAndHeadersBuffer(other._startLineAndHeadersBuffer),
	_bodyBuffer(other._bodyBuffer), _headersParsed(other._headersParsed), _bodyStart(other._bodyStart),
	_clientBytesSent(other._clientBytesSent), _clientResponse(other._clientResponse), _serverFd(other._serverFd),
	_serverManager(other._serverManager), _lastActiveTime(other._lastActiveTime), _closeAfterResponse(other._closeAfterResponse) {
	std::cout << "Copy constructor called for Client" << std::endl;
}

//Setters
void	Client::setLastActiveTime( std::time_t timeStamp) {
	this->_lastActiveTime = timeStamp;
}

// void Client::setIsClosed(bool flag) {
// 	this->_closed = flag;
// }




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

// const std::string&	Client::getClientsResponse(void) const {
// 	return(_clientResponse);
// }

//bool&	Client::getIsClosed(void) {
//	return(_closed);
//}




// std::string	Client::getCgiResponse(Request &request) {

// 	CGIHandler	cgiHandler;
// 	std::cout << "we call CGI" << std::endl;
// 	Response cgiResponse = cgiHandler.handle(request);
// 	// std::vector<char> respVector = cgiResponse.serialize();
// 	std::string respStr(respVector.begin(), respVector.end());

// 	return(respStr);
// }



std::string	Client::prepareResponse(int clientFd) {
	std::cout << "Preparing response for client: " << this->getServerFd() << std::endl;
	int		socketClientConnectedTo = this->getServerFd();
	Response _response(&_request, _serverManager, socketClientConnectedTo, clientFd);
	_response.generateResponse();
	if (getAnyHeader(_response.getHeaders(), "Connection") == "close")
		_closeAfterResponse = true;
	return (_response.getRawResponse());
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
		if (!_headersParsed) {
			_startLineAndHeadersBuffer += incomingData;
			if (!headersComplete(_startLineAndHeadersBuffer)) {
				std::cout << "Request is still incomplete, waiting for more data..." << std::endl;
				return;
			}
			_request.reset();
			_request = Request(_startLineAndHeadersBuffer);
			std::cout << "Trying to parse request for " << clientFd << std::endl;
			try {
				if (!_headersParsed)
					_request.parseRequest();
			} catch(const std::exception& e) {
				std::cerr << "Failed to parse request: "<< e.what() << '\n';
				_serverManager->closeClientFd(clientFd);
				return;
			}
			_headersParsed = true;
		}
		// If headers are parsed, we can now check for the body. It is optional depending on request type, 
		// so it is separated from the headers parsing logic.
		std::cout << "Is body expected? " << (_request.getBodyExpected() ? "Yes" : "No") << std::endl;

		if (_headersParsed && _request.getBodyExpected())
		{
			_bodyBuffer += incomingData.substr(_bodyStart); // we append the chunk that may have been read with headers
			_bodyStart = 0;
			if (bodyComplete(_bodyBuffer)) {
				_request.setBody(_bodyBuffer);
				_request.parseBody();
				_bodyBuffer.clear(); // Clear the body buffer after parsing
				
			} else {
				std::cout << "Body is not complete, waiting for more data..." << std::endl;
				return;
			}
		}
		std::cout << "Request parsed successfully." << std::endl;
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
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
	if (_clientResponse.empty()) {
		_response =std::make_unique<Response>(&_request, _serverManager, _serverFd, clientFd);
		_response->generateResponse();
		if (getAnyHeader(_response->getHeaders(), "Connection") == "close")
			_closeAfterResponse = true;
		_clientResponse = _response->getRawResponse();
		if (_response->getIsCGI()) {
			if (_response->getCgiHandler()->isDone()) {
				std::cout << "CGI handler is done, preparing CGI response." << std::endl;
				_clientResponse = _response->getCgiHandler()->finalize();
				if (_clientResponse.empty()) {
					std::cerr << "Error: CGI response is empty, closing client connection." << std::endl;
					_serverManager->closeClientFd(clientFd);
					return;
				}
				_clientBytesSent = 0; // Reset bytes sent for the new CGI response
			} else {
			std::cerr << "CGI response will follow." << std::endl;
			return;
		}
	}
		if (_clientResponse.empty()) {
			std::cerr << "Error: Response is empty, closing client connection." << std::endl;
			_serverManager->closeClientFd(clientFd);
			return;
		}
		_clientBytesSent = 0; // Reset bytes sent for the new response
		std::cout << "Response prepared successfully." << std::endl;
		// Set the epoll event to EPOLLOUT to send the response		
		}
	const char*	servResp = _clientResponse.c_str();
	size_t		responseSize = _clientResponse.size();
	ssize_t		bytesSent = send(clientFd, servResp + _clientBytesSent, responseSize - _clientBytesSent, 0);
	if (bytesSent == -1) {
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}
	_clientBytesSent += bytesSent;
	if (_clientBytesSent == responseSize) {
		std::cout << "All data sent" << "\n";
		_clientBytesSent = 0;
		_serverManager->closeClientFd(clientFd);	
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
