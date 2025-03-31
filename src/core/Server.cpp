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
	struct sockaddr_storage	clientAddr;
	socklen_t				clientLen = sizeof(clientAddr);
	struct epoll_event	ev, events[MAX_EVENTS];
	int					epollFd, cSock, rfds;

	_isRunning = true;
	std::cout << "Server started. Waiting for connections..." << "\n";

	epollFd = epoll_create(MAX_EVENTS);
	if (epollFd == -1){
		handleError("Error: epoll_create()");
		close(_sockFd);
		exit(EXIT_FAILURE);
	}
	ev.events = EPOLLIN;
	ev.data.fd = _sockFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _sockFd, &ev) == -1) {

		handleError("Error: epoll_ctl");
		close(_sockFd);
		exit(EXIT_FAILURE);
	}

	while (_isRunning)
	{

		rfds = epoll_wait(epollFd, events, MAX_EVENTS, 0);
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
				}

				if (!Server::setNonBlocking(cSock)) {
					std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< "\n";
					close(cSock);
					continue;
				}
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = cSock;
				if (epoll_ctl(epollFd, EPOLL_CTL_ADD, cSock, &ev) == -1) {

					handleError("Error: epoll_ctl");
					close(_sockFd);
					continue;
				}

			}
		}
		// Handle the connection
		handleConnection(cSock);
	}


/*
    Cleans up serverListources:
    1. Sets _is_running flag to false
    2. Closes the server socket
    3. Invalidates the socket descriptor
*/
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

void Server::handleConnection(int cSock)
{
	std::cout << "New connection accepted" << "\n";

	// Buffer for receiving data
	char buffer[1024] = {0};

	// Receive request
	ssize_t bytes_read = recv(cSock, buffer, sizeof(buffer) - 1, 0); // recv - receives data from a socket
	if (bytes_read > 0)
	{
		std::cout << "Received request:\n" << buffer << "\n";
	}

	// Send serverListponse
	const char *serverListponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";  // HTTP serverListponse
	ssize_t bytes_sent = send(cSock, serverListponse, strlen(serverListponse), 0); // send - sends data to a socket
	if (bytes_sent > 0)
	{
		std::cout << "Sent response (" << bytes_sent << " bytes)" << "\n";
	}

	// Wait a bit before closing (for testing)
	usleep(100000); // Wait 100ms

	// CLose the client socket
	close(cSock);
	std::cout << "Connection closed" << "\n";
	}
