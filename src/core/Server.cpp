#include "Server.hpp"
Server::Server(int port, const std::string& host) : _servSockFd(-1), _servPort(port), _servHost(host), _isRunning(false){

}

Server::~Server()
{
	stop();
}

/* 
    This method sets up the server socket:
    1. Creates a TCP socket
    2. Sets socket options (SO_REUSEADDR allows reuse of the addserverLists)
    3. Sets the socket to non-blocking mode
    4. Binds the socket to our addserverLists
    5. Starts listening for connections
    6. Returns true if successful
	*/
bool Server::init()
{
	struct addrinfo		hints, *serverList, *rp;
	int					infoRet;
	
	std::memset(&hints, 0 , sizeof(struct addrinfo));
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	
	std::string strPort = std::to_string(_servPort);

	infoRet = getaddrinfo(_servHost.c_str(), strPort.c_str(), &hints, &serverList);
	if (infoRet != 0 ) {
		std::cerr << "Error: getaddrinfo(): " << gai_strerror(infoRet) << "\n";
		return (false);
	}

	//struct sockaddr_in *ipv4 = (struct sockaddr_in *)serverList->ai_addr;
	//char ip[INET_ADDRSTRLEN];
	//inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
	//std::cout << "IPv4 AddserverLists: " << ip << std::endl;



	for (rp = serverList; rp != NULL; rp = rp->ai_next) {
		
		_servSockFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (_servSockFd == -1)
		{
			continue;
		}
		if (bind(_servSockFd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break;
		}
		close (_servSockFd);
	}

	if (rp == NULL) {
		std::cerr << "Failed to bind to any addserverLists" << std::endl;
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(serverList);


	// Set socket to non-blocking mode
	if (!setNonBlocking(_servSockFd))
	{
		std::cerr << "Error: Failed to set non-blocking mode" << std::endl;
		close(_servSockFd);
		return false;
	}

	// Bind socket
	//

	// Listen to connections
	if (listen(_servSockFd, 10) < 0) // listen - makes the socket listen for incoming connections
	{
		std::cerr << "Error: Listen failed" << std::endl;
		close(_servSockFd);
		return false;
	}

	std::cout << "Server initialized on port " << _servPort << std::endl;
	return true;
}

/*
    This is the main server loop:
    1. Checks if server is initialized
    2. Sets _is_running to true
    3. Enters a loop that:
        - Accepts new connections
        - Handles non-blocking errors
        - Sets client sockets to non-blocking mode
        - Handles each connection
*/
void Server::start()
{
	if (_servSockFd < 0)
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
		int client_socket = accept(_servSockFd, (struct sockaddr *)&client_addserverLists, &client_len);    // accept - accepts incoming connections
		if (client_socket < 0)
		{
			// std::cerr << "Error: Accept failed" << std::endl;
			// perror("Error: Accept failed");
			if (errno == EAGAIN || errno == EWOULDBLOCK) // change this to something that is not errno
				continue;
		}

		// Set client socket to non-blocking mode
		setNonBlocking(client_socket);

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
    if (_servSockFd >= 0)
    {
        close(_servSockFd);
        _servSockFd = -1;
        std::cout << "Server stopped" << std::endl;
    }
}

/*
    Handles individual client connections:
    1. Receives data from client
    2. Send a simple HTTP serverListponse
    3. Waits briefly (for testing)
    4. Closes the connection
*/
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

/*
    Sets a file descriptor to non-blocking mode:
    1. Gets the current flags
    2. Adds the O_NONBLOCK flag
    3. Sets new flags
*/
bool Server::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0); 
    if (flags < 0)
    {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0;
}