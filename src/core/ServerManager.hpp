#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#define EPOLL_CAPACITY				20
#define DEFAULT_CONFIG_FILE_PATH	"./webserv.conf"
#define SERVER_TIMEOUT				50
#define ENABLE						1
#define DISABLE						0
#define NONE						0

#include "parsingConfFile/ParseConfig.hpp"
#include "Server.hpp"
#include "parsingConfFile/vServer.hpp"
#include <fstream>

class Server;

struct Client {

	Client();
	std::string				clientResponse;
	size_t					clientBytesSent;
	time_t					lastActiveTime;
};


class ServerManager {

	private:
		std::ifstream					_configFileFd;
		int								_epollFd;
		std::vector<vServer>			_vServers;
		std::vector<Server>				_servers;
		std::map<int, Client>			_fdClientDataMap;
	
	public:
		//constructors
		ServerManager(std::string& ConfigFileName, int epollSize);
		~ServerManager();

		//getters
		int						getEpollFd( void ) const;
		int						getSocketFd(const vServer& vServer);
		addrinfo*				getAddrList(const vServer& vServer) ;
		std::ifstream&			getConfigFileFd( void );
		std::vector<vServer>&	getVirtualServers( void );
		
		//setter
		void					setServers(const std::vector<vServer>& vSrevers);
		void					setSocketsToEpollIn(void);
		void					setEpollCtl( int targetFd, int eventFlag, int operation);
		bool					setNonBlocking(int fd);

		//methods
		void					parsConfigFile(std::vector<vServer>& _vServers);
		int						bindSocket(addrinfo* addrList);
		void					runServers( void );
		void					manageEpollEvent(const struct epoll_event& epollEvents);
		void					manageListenSocketEvent(const struct epoll_event& epollEvents);
		bool					isListeningSocket(int fd);
		void					addClient(int clientFd);
		void					closeIdleConnections(int socketFd);



		void					readRequest( int clientFd );
		void					sendResponse( int clientFd );
		void					prepResponse(int clientFd );






		class ServerManagerException : public std::exception {
			private:
				std::string	_message;
			public:
				ServerManagerException(const std::string& message);
				const char*	what() const noexcept override;
		};
};

#endif