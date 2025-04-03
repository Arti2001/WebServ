#include "Server.hpp"

Server::Server(std::string port,  std::string host) : _sockFd(-1), _servPort(port), _servHost(host), _isRunning(false){

}

Server::~Server()
{
	stop();
}

bool Server::setNonBlocking(int fd) {

	int prevFlags  = fcntl(fd, F_GETFL);

	if (prevFlags == -1) {
		std::cerr << " Error: fctl(): Failed to get the fd's flags!"<< "\n";
		return (false);
	}
	if (fcntl(fd, F_SETFL, prevFlags | O_NONBLOCK) == -1) {
		std::cerr << " Error: fctl(): Failed to set the fd's flags!"<< "\n";
		return (false);
	}
	return (true);
}

//error handler

void handleError(const std::string msg) {

	std::cerr << msg << ": " << strerror(errno) << "\n";
}



bool Server::init()
{
	struct addrinfo hints, *res, *rp;
	int				infoRet;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	

	infoRet = getaddrinfo(_servHost.c_str(), _servPort.c_str(), &hints, &res);
	if (infoRet != 0 ) {
		std::cerr << "Error: getaddrinfo(): " << gai_strerror(infoRet) << "\n";//we have to return  infoRet
		return (false);
	}


	for (rp = res; rp != NULL; rp = rp->ai_next) {

		_sockFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (_sockFd == -1) {
			continue;
		}
		if (bind(_sockFd, rp->ai_addr, rp->ai_addrlen) == 0) {
			std::cerr << "Successfully binded" << "\n";
			break;
		}
		close (_sockFd);
	}
	freeaddrinfo(res);
	if (rp == NULL) {
		std::cerr << "Failed to bind to any results" << "\n";
		exit(EXIT_FAILURE);
	}
	if (_sockFd < 0)
	{
		std::cerr << "Error: Server not initialized" << "\n";
		return (false);
	}
	if (!Server::setNonBlocking(_sockFd)) {
		std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< "\n";
	}



	if (listen(_sockFd, QUEUE_LENGTH) == -1)
	{
		std::cerr << "Error: Failed to isten" << "\n";
		close(_sockFd);
		return false;
	}
	else
		std::cout << " Server Listening"<< "\n";

	return true;
}


void Server::start()
{
	struct sockaddr_storage		clientAddr;
	socklen_t					clientLen = sizeof(clientAddr);
	struct epoll_event			clientEvent, events[MAX_EVENTS];
	int							cSock = -1, rfds = 0;

	_isRunning = true;
	

	_epollFd = epoll_create(MAX_EVENTS);
	if (_epollFd == -1){
		handleError("Error: epoll_create()");
		close(_sockFd);
		exit(EXIT_FAILURE);
	}

	
	clientEvent.events = EPOLLIN;
	clientEvent.data.fd = _sockFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _sockFd, &clientEvent) == -1) {
		handleError("Error: epoll_ctl");
		close(_sockFd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Server started. Waiting for connections..." << "\n";

	while (_isRunning)
	{
		rfds = epoll_wait(_epollFd, events, MAX_EVENTS, 0);
		if (rfds == -1) {
			handleError("Error: epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < rfds; i++) {


			if (events[i].data.fd == _sockFd) {

				cSock = accept(_sockFd, (struct sockaddr *)&clientAddr, &clientLen);
				if (cSock == -1) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						std::cerr << "No client connections available at the moment." << std::endl;
						continue;
					}
					handleError("Error: accept");
					exit(EXIT_FAILURE);
				}

				if (!Server::setNonBlocking(cSock)) {
						std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< "\n";
						close(cSock);
						continue;
				}

				clientEvent.events = EPOLLIN;
				clientEvent.data.fd = cSock;
				if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, cSock, &clientEvent) == -1) {
	
					handleError("Error: epoll_ctl");
					close(_sockFd);
					exit(EXIT_FAILURE);
				}
			}
			if(events[i].events & EPOLLIN) {
				readRequest(cSock);
			}
			if (events[i].events & EPOLLOUT) {
				sendResponse(cSock);
			}

			
		}
	}
}

void Server::readRequest (int clientFd) {

	char		recBuff[RECBUFF];//8KB
	ssize_t		bytesRead;

	bytesRead = recv(clientFd, recBuff, RECBUFF - 1, 0);

	if (bytesRead > 0)
	{
		recBuff[bytesRead] = '\0';
		std::cout << "######Request#####\n" << recBuff << "\n";

		prepResponse(clientFd);

		return ;
	}
	else if (bytesRead == 0){
		std::cout << "Connection lost" << bytesRead << "\n";
		close(clientFd);
		_clients.erase(clientFd);
		return;
	}
	else
		std::cout << "Nod data to read !"<< "\n";
}

void Server::sendResponse(int clientFd) {

	const char*	servResp = _clients[clientFd].response.c_str();
	struct epoll_event	clientEvent;
	//ssize_t		totalSent = 0;
	ssize_t		bytesSent;	

	
	bytesSent = send(clientFd, servResp, strlen(servResp), 0);
	if (bytesSent == -1) {
		close(clientFd);
		handleError("Error send: ");
		return;
	}
	usleep(1000000);
	std::cout << bytesSent << "\n";
	clientEvent.data.fd = clientFd;
	clientEvent.events = EPOLLIN;
	epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientFd, &clientEvent);
	std::cout << "Connection closed" << "\n";
	close(clientFd);
	return;
}


void	Server::prepResponse(int clientFd) {

	struct epoll_event	clientEvent;

	
	_clients[clientFd].response = HARDCODEDRESP;
	_clients[clientFd].bytesSent = 0;

	clientEvent.data.fd = clientFd;
	clientEvent.events = EPOLLOUT;
	
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientFd, &clientEvent) == -1) {
		handleError("Error: could not modificate the client's fd events!");
		close(_sockFd);
		exit(EXIT_FAILURE);
	}
}

clientInfo& Server::getclientInfo( int clientFd ) {
	
	return _clients[clientFd];
}

void Server::stop()
{
	_isRunning = false;
	if (_sockFd >= 0)
	{
		close(_sockFd);
		_sockFd = -1;
		std::cout << "Server stopped" << "\n";
	} 
}

