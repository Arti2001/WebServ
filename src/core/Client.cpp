#include "Client.hpp" 
#include "parsingRequest/RequestParser.hpp"


Client::Client(int serverFd, ServerManager* serverManager) : _clientBytesSent(0),
	_serverFd(serverFd), _serverManager(serverManager){
	}



Client::~Client() {
}

//Setters
//void	Client::setLastActiveTime( std::time_t timeStamp) {
//	this->_lastActiveTime = timeStamp;
//}

//void Client::setIsClosed(bool flag) {
//	this->_closed = flag;
//}




//Getters

//std::time_t	Client::getLastActiveTime( void ) const {

//	return(_lastActiveTime);
//}

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




std::string	Client::getCgiResponse(HTTPRequest request) {

	CGIHandler	cgiHandler;
	std::cout << "we call CGI" << std::endl;
	Response cgiResponse = cgiHandler.handle(request);
	std::vector<char> respVector = cgiResponse.serialize();
	std::string respStr(respVector.begin(), respVector.end());

	return(respStr);
}


std::string	Client::getResponse(HTTPRequest request) {

	int									socketClientConnectedTo = this->getServerFd();
	const std::vector<const vServer*>&	subServConfigs = _serverManager->findServerCofigsByFd(socketClientConnectedTo);
	std::string							hostHeaderValue = getAnyHeader(request.getHeaders(), "Host");
	const vServer&						askedServConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);
	const Location&						askedLocationBlock = _serverManager->findLocationBlockByUrl(askedServConfig, request.getUri());
	std::cout << " URI is: " + request.getUri()<< "\n";
	std::cout << " Returnd location is: " + askedLocationBlock._locationPath << "\n";


	StaticHandler handler;
	Response response= handler.serve(request, askedLocationBlock);

	std::vector<char> respVector = response.serialize();
	std::string respStr(respVector.begin(), respVector.end());
	return (respStr);
}


void Client::addToRequestBuff(char* chunk, size_t bytesRead){

std::string strChunk(chunk);
_requestBuffer.append(chunk, bytesRead);
}



//curl -v -H "Host: server3.com" http://127.0.0.1:8055/

// the logic should be following:
// 1. Read the request from the client, save the raw request as a string in Request class
// 2. Parse the request to extract the URI (path and query), headers and body, including chunked transfer encoding if applicable
// 3. Determine and save the target server and location block based on the "host" header
// 


void    Client::readRawRequest (int clientFd) {
    
    char        requestBuffer[REQUEST_READ_BUFFER];//8KB
    ssize_t     bytesRead;
    
    bytesRead = recv(clientFd, requestBuffer, REQUEST_READ_BUFFER, 0);
    
    if (bytesRead > 0)
    {
        requestBuffer[bytesRead] = '\0';
		_accumulatedRequest.append(requestBuffer, bytesRead);
		if (!isRequestComplete(_accumulatedRequest)) {
			std::cout << "Request is still incomplete, waiting for more data..." << "\n";
			return;
		}
		Request request(_accumulatedRequest);
		request.parseRequest();
		_accumulatedRequest.clear(); // Clear the buffer after processing
		Response response(request, _serverManager, clientFd)
		generateResponse(); 
        this->_clientResponse = response.getResponse();
        _serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
        return ;
    }
    else if (bytesRead == 0){
        std::cout << "Client disconnected: Clean up!" << bytesRead << "\n";
        _serverManager->closeClientFd(clientFd);
        return;
    }
    else
        std::cout << "No data to read !"<< "\n";
}




void	Client::sendResponse(int clientFd) {
	
	size_t&		totalBytesSent = this->getClientsBytesSent();
	const char*	servResp = this->getClientsResponse().c_str();
	size_t		responseSize = strlen(servResp);
	ssize_t		bytesSent = send(clientFd, servResp + totalBytesSent, responseSize - totalBytesSent, 0);

	if (bytesSent == -1) {
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}
	totalBytesSent += bytesSent;
	if (totalBytesSent == strlen(servResp)) {
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
