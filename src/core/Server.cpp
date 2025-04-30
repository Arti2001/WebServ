#include "Server.hpp"

Server::Server(std::string port,  std::string host) : _sockFd(-1), _epollFd(-1), _servPort(port), _servHost(host), _isRunning(false){

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
	//if (_sockFd < 0)
	//{
	//	std::cerr << "Error: Server not initialized" << "\n";
	//	return (false);
	//}
	if (!Server::setNonBlocking(_sockFd)) {
		std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< "\n";
	}



	if (listen(_sockFd, QUEUE_LENGTH) == -1)
	{
		std::cerr << "Error: Failed to isten" << "\n";
		close(_sockFd);
		return false;
	}
	
	std::cout << " Server Listening"<< "\n";

	_isRunning = true;
	return true;
}


void Server::start()
{
	struct sockaddr_storage		clientAddr;
	socklen_t					clientLen = sizeof(clientAddr);
	struct epoll_event			events[MAX_EVENTS];
	int							cSock = -1, rfds = 0;

	

	_epollFd = epoll_create(MAX_EVENTS);
	if (_epollFd == -1){
		handleError("Error: epoll_create()");
		close(_sockFd);
		exit(EXIT_FAILURE);
	}

	setEvent(_sockFd, IN, EPOLL_CTL_ADD);

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
				setEvent(cSock, IN, EPOLL_CTL_ADD);
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
		setEvent(clientFd, OUT, EPOLL_CTL_MOD);
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

	size_t&		totalSent = _clients[clientFd].bytesSent;
	const char*	servResp = _clients[clientFd].response.c_str();
	ssize_t		bytesSent;	

	
	bytesSent = send(clientFd, servResp + totalSent, strlen(servResp) - totalSent, 0);
	if (bytesSent == -1) {
		setEvent(clientFd, OUT, EPOLL_CTL_MOD);
		handleError("Error send: ");
		return;
	}
	totalSent += bytesSent;
	if (totalSent == strlen(servResp)) {
		std::cout << "All data sent: Connection closed" << "\n";
		setEvent(clientFd, IN, EPOLL_CTL_MOD);
		close(clientFd);
		totalSent = 0;
		_clients[clientFd].response.clear();
	}
}


void	Server::prepResponse(int clientFd) {

	_clients[clientFd].response = "lol";
	_clients[clientFd].bytesSent = 0;
}

void Server::setEvent(int clientFd, int evFlag, int op) {

	struct epoll_event	clientEvent;

	clientEvent.data.fd = clientFd;
	if (evFlag == OUT)
		clientEvent.events = EPOLLOUT;
	else
		clientEvent.events = EPOLLIN;
	
	if (epoll_ctl(_epollFd, op, clientFd, &clientEvent) == -1) {
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

