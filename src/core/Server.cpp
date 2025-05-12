#include "Server.hpp"

Server::Server(int socketFd, const vServer& vServer) : _vServer(vServer){
	this->_socketFd = socketFd;

	std::string ip = _vServer.getServerIp();

}
Server::~Server() {}

int	Server::getSocketFd(void) const {
	return (_socketFd);
}





//void Server::start()
//{
//	struct sockaddr_storage		clientAddr;
//	socklen_t					clientLen = sizeof(clientAddr);
//	struct epoll_event			events[MAX_EVENTS];
//	int							cSock = -1, rfds = 0;

	

//	_epollFd = epoll_create(MAX_EVENTS);
//	if (_epollFd == -1){
//		handleError("Error: epoll_create()");
//		close(_socketFd);
//		exit(EXIT_FAILURE);
//	}

//	setEpollEvent(_socketFd, IN, EPOLL_CTL_ADD);

//	std::cout << "Server started. Waiting for connections..." << "\n";

//	while (_isRunning)
//	{
//		rfds = epoll_wait(_epollFd, events, MAX_EVENTS, 0);
//		if (rfds == -1) {
//			handleError("Error: epoll_wait");
//			exit(EXIT_FAILURE);
//		}

//		for (int i = 0; i < rfds; i++) {


//			if (events[i].data.fd == _socketFd) {

//				cSock = accept(_socketFd, (struct sockaddr *)&clientAddr, &clientLen);
//				if (cSock == -1) {
//					if (errno == EAGAIN || errno == EWOULDBLOCK) {
//						std::cerr << "No client connections available at the moment." << std::endl;
//						continue;
//					}
//					handleError("Error: accept");
//					exit(EXIT_FAILURE);
//				}

//				if (!Server::setNonBlocking(cSock)) {
//						std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< "\n";
//						close(cSock);
//						continue;
//				}
//				setEpollEvent(cSock, IN, EPOLL_CTL_ADD);
//			}
//			if(events[i].events & EPOLLIN) {
//				readRequest(cSock);
//			}
//			if (events[i].events & EPOLLOUT) {
//				sendResponse(cSock);
//			}

			
//		}
//	}
//}

//void Server::readRequest (int clientFd) {

//	char		recBuff[RECBUFF];//8KB
//	ssize_t		bytesRead;

//	bytesRead = recv(clientFd, recBuff, RECBUFF - 1, 0);

//	if (bytesRead > 0)
//	{
//		recBuff[bytesRead] = '\0';
//		std::cout << "######Request#####\n" << recBuff << "\n";

//		prepResponse(clientFd);
//		setEpollEvent(clientFd, OUT, EPOLL_CTL_MOD);
//		return ;
//	}
//	else if (bytesRead == 0){
//		std::cout << "Connection lost" << bytesRead << "\n";
//		close(clientFd);
//		_clients.erase(clientFd);
//		return;
//	}
//	else
//		std::cout << "Nod data to read !"<< "\n";
//}

//void Server::sendResponse(int clientFd) {

//	size_t&		totalSent = _clients[clientFd].bytesSent;
//	const char*	servResp = _clients[clientFd].response.c_str();
//	ssize_t		bytesSent;	

	
//	bytesSent = send(clientFd, servResp + totalSent, strlen(servResp) - totalSent, 0);
//	if (bytesSent == -1) {
//		setEpollEvent(clientFd, OUT, EPOLL_CTL_MOD);
//		handleError("Error send: ");
//		return;
//	}
//	totalSent += bytesSent;
//	if (totalSent == strlen(servResp)) {
//		std::cout << "All data sent: Connection closed" << "\n";
//		setEpollEvent(clientFd, IN, EPOLL_CTL_MOD);
//		close(clientFd);
//		totalSent = 0;
//		_clients[clientFd].response.clear();
//	}
//}


//void	Server::prepResponse(int clientFd) {

//	_clients[clientFd].response = "lol";
//	_clients[clientFd].bytesSent = 0;
//}

//void Server::setEpollEvent(int clientFd, int evFlag, int op) {

//	struct epoll_event	clientEvent;

//	clientEvent.data.fd = clientFd;
//	if (evFlag == OUT)
//		clientEvent.events = EPOLLOUT;
//	else
//		clientEvent.events = EPOLLIN;
	
//	if (epoll_ctl(_epollFd, op, clientFd, &clientEvent) == -1) {
//		handleError("Error: could not modificate the client's fd events!");
//		close(_socketFd);
//		exit(EXIT_FAILURE);
//	}
//}

//clientInfo& Server::getclientInfo( int clientFd ) {
	
//	return _clients[clientFd];
//}

//void Server::stop()
//{
//	_isRunning = false;
//	if (_socketFd >= 0)
//	{
//		close(_socketFd);
//		_socketFd = -1;
//		std::cout << "Server stopped" << "\n";
//	} 
//}

