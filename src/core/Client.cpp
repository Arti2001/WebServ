/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:07:50 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/07/08 09:16:04 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp" 


Client::Client(int serverFd, ServerManager* serverManager) : _headersParsed(false), _clientBytesSent(0),
	_serverFd(serverFd), _serverManager(serverManager), _closeAfterResponse(false){
	}



Client::~Client() {
}

Client::Client(const Client& other) : _request(other._request), _startLineAndHeadersBuffer(other._startLineAndHeadersBuffer),
	_bodyBuffer(other._bodyBuffer), _headersParsed(other._headersParsed), _bodyStart(other._bodyStart),
	_clientBytesSent(other._clientBytesSent), _clientResponse(other._clientResponse), _serverFd(other._serverFd),
	_serverManager(other._serverManager), _lastActiveTime(other._lastActiveTime), _closeAfterResponse(other._closeAfterResponse) {
}

//Setters
void	Client::setLastActiveTime( std::time_t timeStamp) {
	this->_lastActiveTime = timeStamp;
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

std::string	Client::prepareResponse(int clientFd) {
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
				std::cout << "Headers not complete yet, waiting for more data..." << std::endl;
				return;
			}
			_request.reset();
			_request = Request(_startLineAndHeadersBuffer);
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
		if (_headersParsed && _request.getBodyExpected() && _request.getStatusCode() < 400)
		{
			_bodyBuffer += incomingData.substr(_bodyStart); // we append the chunk that may have been read with headers
			_bodyStart = 0;
			if (bodyComplete(_bodyBuffer)) {
				_request.setBody(_bodyBuffer);
				_request.parseBody();
				_bodyBuffer.clear(); // Clear the body buffer after parsing
				
			} else {
				// std::cout << "Body not complete yet, waiting for more data..." << std::endl;
				return;
			}
		}
		_startLineAndHeadersBuffer.clear(); // Clear the buffer after parsing headers and body
		_headersParsed = false; // Reset the headers parsed flag for the next request
		std::cout << "Request start line: " << _request.getPath() << std::endl;
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
        return ;
		
    }
    else if (bytesRead == 0) {
        std::cout << "Client disconnected: Clean up!" << std::endl;
        _serverManager->closeClientFd(clientFd);
        return;
    } else {
        std::cout << "Recv failed"<< std::endl;
		_serverManager->closeClientFd(clientFd);
		return;
	}
}


void	Client::handleResponse(int clientFd) {
	if (_clientResponse.empty()) {
		_response =std::make_unique<Response>(&_request, _serverManager, _serverFd, clientFd);
		_response->generateResponse();
		if (getAnyHeader(_response->getHeaders(), "Connection") == "close")
			_closeAfterResponse = true;
		_clientResponse = _response->getRawResponse();
		if (_response->getIsCGI()) {
			// std::cout << "CGI is coming" << std::endl;
			_serverManager->setEpollCtl(clientFd, EPOLLIN, EPOLL_CTL_MOD);
			return;
		}
		if (_clientResponse.empty()) {
			std::cerr << "Error: Response is empty, closing client connection." << std::endl;
			_serverManager->closeClientFd(clientFd);
			return;
		}
		_clientBytesSent = 0; // Reset bytes sent for the new response
	}
	sendResponse(_clientResponse, clientFd);
}

void Client::sendResponse(std::string responseBody, int clientFd)
{
	const char*	servResp = responseBody.c_str();
	size_t		responseSize = responseBody.size();
	ssize_t		bytesSent = send(clientFd, servResp + _clientBytesSent, responseSize - _clientBytesSent, 0);
	if (bytesSent == -1) {
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}
	_clientBytesSent += bytesSent;
	if (_clientBytesSent == responseSize) {
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
