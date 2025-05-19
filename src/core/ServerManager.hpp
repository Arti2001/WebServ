#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#define EPOLL_CAPACITY				20
#define DEFAULT_CONFIG_FILE_PATH	"./webserv.conf"
#define SERVER_TIMEOUT				10
#define ENABLE						1
#define DISABLE						0
#define NONE						0

#include "parsingConfFile/ParseConfig.hpp"
#include "Server.hpp"
#include "parsingConfFile/vServer.hpp"
#include <fstream>
#include "parsingRequest/HTTPRequest.hpp"
#include "parsingResponse/StaticHandler.hpp"

class Server;

struct Client {

	Client();
	std::string				clientResponse;
	size_t					clientBytesSent;
	time_t					lastActiveTime;
	int						serverFd;
};


class ServerManager {

	private:
		std::ifstream										_configFileFd;
		int													_epollFd;
		std::vector<vServer>								_vServers;
		std::map<std::string, std::vector<const vServer*>>	_hostSetMap;
		std::vector<Server>									_servers;
		std::map<int, Client>								_fdClientDataMap;
	
	public:
		//constructors
		ServerManager(std::string& ConfigFileName, int epollSize);
		~ServerManager();

		//getters
		int						getEpollFd( void ) const;
		int						getSocketFd(const std::string& host, const std::string& port);
		addrinfo*				getAddrList(const std::string& host, const std::string& port) ;
		std::ifstream&			getConfigFileFd( void );
		std::vector<vServer>&	getVirtualServers( void );
		
		//setter
		void					setServers();
		//void					setServerSettingsMap(const std::vector<vServer>& _vServers);
		void					setSocketsToEpollIn(void);
		void					setEpollCtl( int targetFd, int eventFlag, int operation);
		bool					setNonBlocking(int fd);

		//methods
		void								groupServers(const std::vector<vServer>& _vServers);
		void								parsConfigFile(std::vector<vServer>& _vServers);
		int									bindSocket(addrinfo* addrList);
		void								runServers( void );
		void								manageEpollEvent(const struct epoll_event& epollEvents);
		void								manageListenSocketEvent(const struct epoll_event& epollEvents);
		bool								isListeningSocket(int fd);
		void								addClient(int clientFd, int serverFd);
		void								closeIdleConnections();
		void								closeClientFd(int clientfFd);
		const vServer&						findServerConfigByName(const std::vector<vServer>& subConfigs, std::string serverName);
		const std::vector<vServer>&			findServerCofigsByFd(int serverFd);


		void					readRequest( int clientFd );
		void					sendResponse( int clientFd );		
		void					prepResponse(int clientFd );


		//tmpFunctions
		std::string	getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName);






		class ServerManagerException : public std::exception {
			private:
				std::string	_message;
			public:
				ServerManagerException(const std::string& message);
				const char*	what() const noexcept override;
		};
};

#endif