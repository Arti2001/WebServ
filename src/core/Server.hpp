#ifndef SERVER_HPP
#define SERVER_HPP

#define MAX_EVENTS 10 

#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>


class Server
{
    private:
        int                 _sockFd;
        std::string         _servPort;
        std::string         _servHost;
        bool                _isRunning;

    public:
        Server(std::string port, std::string host);
      //  Server(const Server& other);
      //  Server& operator=(const Server& other);
        ~Server();

        bool init();//to change
        bool setNonBlocking(int fd);
        void start();
        void stop();
        bool isRunning() const;

    private:
        void handleConnection(int client_socket);
};

#endif