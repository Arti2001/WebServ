#ifndef SERVER_HPP
#define SERVER_HPP

#define QUEUE_LENGTH	10
#define RECBUFF			8192
#define IN				1
#define OUT				2


#include <string>
#include <sstream>
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
#include <exception>
#include <map>
#include <vector>
#include <sstream>
#include <bits/stdc++.h>
#include "ServerManager.hpp"

#include "parsingConfFile/ParseConfig.hpp"


struct clientInfo {

	std::string	response;
	size_t		bytesSent = 0;
};

class Server
{
	private:
		int							_sockFd;
		int							_epollFd;
		std::string					_serverPort;
		std::string					_serverHost;
		bool						_isRunning;
		std::map<int, clientInfo>	_clients;

		public:
		Server(const vServer&	serverSet);
		//  Server(const Server& other);
		//  Server& operator=(const Server& other);
		~Server();
		
		void readRequest( int clientFd );
		void sendResponse( int clientFd );
		bool		init();//to change
		//bool		setNonBlocking(int fd);
		void		start();
		void		stop();
		bool		isRunning() const;
		//clientInfo&	getclientInfo( int clientFd);
		void		prepResponse(int clientFd);
		void		setEpollEvent(int clientFd, int evFlag, int op);
		
	};

#endif