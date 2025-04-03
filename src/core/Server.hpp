#ifndef SERVER_HPP
#define SERVER_HPP

#define MAX_EVENTS		10
#define QUEUE_LENGTH	10
#define RECBUFF			8192

#define HARDCODEDRESP "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!"


#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <csignal>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <map>

struct clientInfo {

	std::string	response;
	size_t		bytesSent = 0;
};

class Server
{
	private:
		int							_sockFd;
		int							_epollFd;
		std::string					_servPort;
		std::string					_servHost;
		bool						_isRunning;
		std::map<int, clientInfo>	_clients;

	public:
		Server(std::string port, std::string host);
		//  Server(const Server& other);
		//  Server& operator=(const Server& other);
		~Server();

		bool		init();//to change
		bool		setNonBlocking(int fd);
		void		start();
		void		stop();
		bool		isRunning() const;
		clientInfo&	getclientInfo( int clientFd);
		void		prepResponse(int clientFd);
		
		private:
			void readRequest( int clientFd );
			void sendResponse( int clientFd );
		
	};

#endif