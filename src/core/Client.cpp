#include "Client.hpp" 


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

void	Client::readRequest (int clientFd) {

	char		recBuff[RECBUFF];//8KB
	ssize_t		bytesRead;
	
	bytesRead = recv(clientFd, recBuff, RECBUFF - 1, 0);
	if (bytesRead > 0) {
		addToRequestBuff(recBuff, bytesRead);
		RequestParser									RequestParser;
		const std::unordered_map<int , HTTPRequest>&	parsedRequest = RequestParser.handleIncomingRequest(clientFd, recBuff);
		std::string										uri;

		if (parsedRequest.find(clientFd) != parsedRequest.end())
			uri = parsedRequest.at(clientFd).getUri();
		else
			throw ServerManager::ServerManagerException("Key is not found");

		if (CGIHandler::isCGIRequest(uri)) {
			this->_clientResponse = getCgiResponse(parsedRequest.at(clientFd));
		}
		else{
			this->_clientResponse = getResponse(parsedRequest.at(clientFd));
		}
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return ;
	}
	else if (bytesRead == 0){

		std::cout << "Client disconnected: Clean up!" << bytesRead << "\n";
		_serverManager->closeClientFd(clientFd);
		return;
	}
	else {
		std::cout << "Not data to read !"<< "\n";
	}
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
