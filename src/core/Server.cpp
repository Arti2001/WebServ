#include "Server.hpp"
#include <cstring>

Server::Server(int port, const std::string &host) : _server_fd(-1), _port(port), _host(host), _is_running(false)
{
    // Initialise the address structure
    memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;              // sin_family - a field that specifies the address family, which determines the format of the address. AF_INET - a constant that represents the IPv4 address family
    _address.sin_addr.s_addr = INADDR_ANY;      // sin_addr - a field that contains the IP address of the socket, s_addr - a field that represents the IP address, INADDR_ANY - a constant that represents "any" IP address, which means that the server will listenon all available network interfaces
    _address.sin_port = htons(_port);           // sin_port - a field that contains the port number of the socket, htons - a function that converts a port number from host byte order to network byte order           
}

Server::~Server()
{
    stop();
}

/*
    This method sets up the server socket:
    1. Creates a TCP socket
    2. Sets socket options (SO_REUSEADDR allows reuse of the address)
    3. Sets the socket to non-blocking mode
    4. Binds the socket to our address
    5. Starts listening for connections
    6. Returns true if successful
*/
bool Server::init()
{
    // Create socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - address family for IPv4, SOCK_STREAM - socket type for TCP, 0 - default protocol
    if (_server_fd < 0)
    {
        std::cerr << "Error: Socket creation failed" << std::endl;
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) // SOL_SOCKET - socket level, SO_REUSEADDR - socket option for enabling address reuse
    {
        std::cerr << "Error: setsockopt failed" << std::endl;
        close(_server_fd);
        return false;
    }

    // Set socket to non-blocking mode
    if (!setNonBlocking(_server_fd))
    {
        std::cerr << "Error: Failed to set non-blocking mode" << std::endl;
        close(_server_fd);
        return false;
    }

    // Bind socket
    if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0) // bind - binds a socket to an address
    {
        std::cerr << "Error: Bind failed" << std::endl;
        close(_server_fd);
        return false;
    }

    // Listen to connections
    if (listen(_server_fd, 10) < 0) // listen - makes the socket listen for incoming connections
    {
        std::cerr << "Error: Listen failed" << std::endl;
        close(_server_fd);
        return false;
    }

    std::cout << "Server initialized on port " << _port << std::endl;
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
    if (_server_fd < 0)
    {
        std::cerr << "Error: Server not initialized" << std::endl;
        return;
    }

    _is_running = true;
    std::cout << "Server started. Waiting for connections..." << std::endl;

    while (_is_running)
    {
        struct sockaddr_in client_address;              // Structure to store client's address
        socklen_t client_len = sizeof(client_address);  // Size of client's address

        // Accept new connection
        int client_socket = accept(_server_fd, (struct sockaddr *)&client_address, &client_len);    // accept - accepts incoming connections
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
    Cleans up resources:
    1. Sets _is_running flag to false
    2. Closes the server socket
    3. Invalidates the socket descriptor
*/
void Server::stop()
{
    _is_running = false;
    if (_server_fd >= 0)
    {
        close(_server_fd);
        _server_fd = -1;
        std::cout << "Server stopped" << std::endl;
    }
}

/*
    Handles individual client connections:
    1. Receives data from client
    2. Send a simple HTTP response
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

    // Send response
    const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";  // HTTP response
    ssize_t bytes_sent = send(client_socket, response, strlen(response), 0); // send - sends data to a socket
    if (bytes_sent > 0)
    {
        std::cout << "Sent response (" << bytes_sent << " bytes)" << std::endl;
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