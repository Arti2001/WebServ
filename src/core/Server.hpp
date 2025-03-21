/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/21 10:19:09 by pminialg      #+#    #+#                 */
/*   Updated: 2025/03/21 10:33:29 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string> // for std::string
#include <sys/socket.h> // for socket-related functions and structures
#include <netinet/in.h> // for internet address structures (like sockaddr_in)
#include <unistd.h> // for system calls like close(), read(), write()
#include <fcntl.h> // for file control operations (like setting non-blocking mode)
#include <iostream> // for console input/output

class Server
{
private:
    int _server_fd;                 // Server socket file descriptor
    struct sockaddr_in _address;    // Structure holding server's address information (such as IP and port)
    int _port;                      // The port number the server listens on
    std::string _host;              // The host address (IP) the server binds to
    bool _is_running;               // Boolean flag to control the server's main loop

public:
    Server(int port = 8080, const std::string &host = "127.0.0.1"); // Constructor - creates a new server with optional port (default 8080) and host (default "127.0.0.1" or localhost)

    ~Server();                                                      // Destructor - cleans up resources when the server is destroyed

    bool init();                                                    // Sets up the server (creates socket, binds to address, starts listening)

    void start();                                                   // Starts the main server loop, which accepts incoming connections, handles each connection, and continues until interrupted
    
    void stop();                                                    // Stops the server's main loop and cleans up resources
    
    bool isRunning() const;                                         // Returns the current running state of the server

private:
    void handleConnection(int client_socket);                       // Handles a single client connection

    bool setNonBlocking(int fd);                                    // Configures asocket to work in non-blocking mode
};

#endif