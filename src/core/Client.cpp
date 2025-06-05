#include "Client.hpp" 


Client::Client(int serverFd, ServerManager* serverManager) : _clientBytesSent(0), _lastActiveTime(std::time(nullptr)),
	_serverFd(serverFd), _serverManager(serverManager), _closed(false) {
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




const Response&	Client::getCgiResponse(HTTPRequest request) {

	CGIHandler	cgiHandler;

	Response cgiResponse = cgiHandler.handle(request);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
}

const Response& Client::getResponse(HTTPRequest request, int clientFd) {


}

void	Client::readRequest (int clientFd) {
	
	char		recBuff[RECBUFF];//8KB
	ssize_t		bytesRead;
	
	bytesRead = recv(clientFd, recBuff, RECBUFF - 1, 0);
	
	if (bytesRead > 0)
	{
		recBuff[bytesRead] = '\0';

		RequestParser	RequestParser;

		const std::unordered_map<int , HTTPRequest>& parsedRequest = RequestParser.handleIncomingRequest(clientFd, recBuff);

		std::string hostHeaderValue = getAnyHeader(parsedRequest.at(clientFd).getHeaders(), "Host");
		//curl -v -H "Host: server3.com" http://127.0.0.1:8055/
		const std::string& uri = parsedRequest.at(clientFd).getUri();

		if (CGIHandler::isCGIRequest(uri)) {

			getCgiResponse(parsedRequest.at(clientFd));
		}
		else{
			getResponse(parsedRequest.at(clientFd), int clientFd);
		}











		int	socketClientConnectedTo = this->getServerFd();
		const std::vector<const vServer*>& subServConfigs = _serverManager->findServerCofigsByFd(socketClientConnectedTo);
		const vServer&	askedServConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);

		//std::cout<< "Asked server is: " << askedServConfig.getServerNames().at(0)<<"\n";
		
		const Location& askedLocationBlock = _serverManager->findLocationBlockByUrl(askedServConfig, uri);

		std::cout << " URI is: " + uri<< "\n";
		std::cout << " Returnd location is: " + askedLocationBlock._locationPath << "\n";


		StaticHandler handler;
		Response response= handler.serve(parsedRequest.at(clientFd), askedLocationBlock);
	
		std::vector<char> respVector = response.serialize();
		std::string respStr(respVector.begin(), respVector.end());
		this->_clientResponse = respStr;
		
		_serverManager->setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return ;
	}
	else if (bytesRead == 0){
		std::cout << "Client disconnected: Clean up!" << bytesRead << "\n";
		_serverManager->closeClientFd(clientFd);
		return;
	}
	else
		std::cout << "Nod data to read !"<< "\n";
}




void	Client::sendResponse(int clientFd) {

	size_t&		totalBytesSent = this->getClientsBytesSent();
	const char*	servResp = this->getClientsResponse().c_str();
	size_t		responseSize = strlen(servResp);
	ssize_t bytesSent = send(clientFd, servResp + totalBytesSent, responseSize - totalBytesSent, 0);

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
