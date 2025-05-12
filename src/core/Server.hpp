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
//#include "ServerManager.hpp"

#include "parsingConfFile/ParseConfig.hpp"



struct clientInfo {

	std::string	response;
	size_t		bytesSent = 0;
};

class Server
{
	private:
		int							_socketFd;
		const vServer&				_vServer;

		public:
			Server(int	socketFd, const vServer& vServer);
			~Server();

			//getters
			int			getSocketFd( void ) const;
		
			//void readRequest( int clientFd );
			//void sendResponse( int clientFd );
	};

#endif