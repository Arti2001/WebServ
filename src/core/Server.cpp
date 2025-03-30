#include "Server.hpp"

Server::Server(std::string port,  std::string host) : _sockFd(-1), _servPort(port), _servHost(host), _isRunning(false){

}

Server::~Server()
{
	stop();
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
		std::cerr << "Error: getaddrinfo(): " << gai_strerror(infoRet) << "\n";
		return (false);
	}

	//struct sockaddr_in *ipv4 = (struct sockaddr_in *)serverList->ai_addr;
	//char ip[INET_ADDRSTRLEN];
	//inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
	//std::cout << "IPv4 AddserverLists: " << ip << std::endl;



	for (rp = res; rp != NULL; rp = rp->ai_next) {

		_sockFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (_sockFd == -1) {
			continue;
		}
		if (bind(_sockFd, rp->ai_addr, rp->ai_addrlen) == 0) {
			std::cerr << "Successfully binded" << std::endl;
			break;
		}
		close (_sockFd);
	}

	if (rp == NULL) {
		std::cerr << "Failed to bind to any results" << std::endl;
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);


	if (!Server::setNonBlocking(_sockFd)) {
		std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< std::endl;
	}




	if (listen(_sockFd, 10) < 0)
	{
		std::cerr << "Error: Listen failed" << std::endl;
		close(_sockFd);
		return false;
	}
	else {
		std::cout << " Server Listening"<< std::endl;
	}




	std::cout << "Server initialized on port " << _servPort << std::endl;
	return true;
}

bool Server::setNonBlocking(int fd) {

	int prevFlags  = fcntl(fd, F_GETFL);

	if (prevFlags == -1) {
		std::cerr << " Error: fctl(): Failed to get the fd's flags!"<< std::endl;
		return (false);
	}
	if (fcntl(fd, F_SETFL, prevFlags | O_NONBLOCK) == -1) {
		std::cerr << " Error: fctl(): Failed to set the fd's flags!"<< std::endl;
		return (false);
	}
	return (true);
}

void Server::start()
{
	if (_sockFd < 0)
	{
		std::cerr << "Error: Server not initialized" << std::endl;
		return;
	}

	_isRunning = true;
	std::cout << "Server started. Waiting for connections..." << std::endl;

	while (_isRunning)
	{
		struct sockaddr_in client_addserverLists;              // Structure to store client's addserverLists
		socklen_t client_len = sizeof(client_addserverLists);  // Size of client's addserverLists

		// Accept new connection
		int client_socket = accept(_sockFd, (struct sockaddr *)&client_addserverLists, &client_len);    // accept - accepts incoming connections
		if (client_socket < 0)
		{
			// std::cerr << "Error: Accept failed" << std::endl;
			// perror("Error: Accept failed");
			if (errno == EAGAIN || errno == EWOULDBLOCK) // change this to something that is not errno
				continue;
		}
		if (!Server::setNonBlocking(client_socket)) {
			std::cerr << "Error: Faild to set fd to a Non Blocking mode!"<< std::endl;
		}
		// Handle the connection
		handleConnection(client_socket);
    }
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
        std::cout << "Server stopped" << std::endl;
    }
}

void Server::handleConnection(int client_socket)
{
	std::cout << "New connection accepted" << std::endl;

	// Buffer for receiving data
	char buffer[1024] = {0};

	// Receive request
	ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0); // recv - receives data from a socket
	if (bytes_read > 0)
	{
		std::cout << "Received request:\n" << buffer << std::endl;
	}

	// Send serverListponse
	const char *serverListponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";  // HTTP serverListponse
	ssize_t bytes_sent = send(client_socket, serverListponse, strlen(serverListponse), 0); // send - sends data to a socket
	if (bytes_sent > 0)
	{
		std::cout << "Sent serverListponse (" << bytes_sent << " bytes)" << std::endl;
	}

	// Wait a bit before closing (for testing)
	usleep(100000); // Wait 100ms

	// CLose the client socket
	close(client_socket);
	std::cout << "Connection closed" << std::endl;
	}
