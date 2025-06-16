#ifndef SERVER_HPP
#define SERVER_HPP

#define QUEUE_LENGTH	10
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




class Server
{
	private:
		int										_socketFd;
		std::vector<const vServer*>				_servConfigs;

		public:
			Server(int	socketFd, std::vector<const vServer*>& vServers);
			~Server();

			//getters
			int								getSocketFd( void ) const;
			const std::vector<const vServer*>&		getServConfigs( void ) const ;
	};

#endif