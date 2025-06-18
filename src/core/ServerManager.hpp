#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#define EPOLL_CAPACITY				20
#define DEFAULT_CONFIG_FILE_PATH	"./webserv.conf"
//#define SERVER_TIMEOUT				10
//constexpr long	SERVER_TIMEOUT_MS = SERVER_TIMEOUT * 1000;
#define ENABLE						1
#define DISABLE						0
#define NONE						0

#include "parsingConfFile/ParseConfig.hpp"
#include "Server.hpp"
#include "parsingConfFile/vServer.hpp"
#include <fstream>
#include "Request/Request.hpp"
#include "Client.hpp"
#include "CGIHandler/CGIHandler.hpp"


extern volatile sig_atomic_t running;

class Server;
class Client;


class ServerManager {

	private:
		std::ifstream										_configFileFd;
		int													_epollFd;
		std::vector<vServer>								_vServers;
		std::map<std::string, std::vector<const vServer*>>	_hostVserverMap;
		std::vector<Server>									_servers;
		std::map<int, Client>								_fdClientMap;
	
	
	public:
		//constructors
		ServerManager(std::string& ConfigFileName, int epollSize);
		~ServerManager();

		//getters
		std::ifstream&			getConfigFileFd( void );
		int						getEpollFd( void ) const;
		int						getSocketFd(const std::string& host, const std::string& port);
		addrinfo*				getAddrList(const std::string& host, const std::string& port) ;
		std::vector<vServer>&	getVirtualServers( void );
		std::map<int, Client>&	getFdClientMap( void );
		std::vector<Server>&	getServers(void);
		
		//setter
		void					setServers();
		void					setSocketsToEpollIn(void);
		void					setEpollCtl( int targetFd, int eventFlag, int operation);
		bool					setNonBlocking(int fd);

		//methods
		void								parsConfigFile(std::vector<vServer>& _vServers);
		void								runServers( void );
		int									bindSocket(addrinfo* addrList);
		void								groupServers(const std::vector<vServer>& _vServers);
		void								manageListenSocketEvent(const struct epoll_event& epollEvents);
		void								manageEpollEvent(const struct epoll_event& epollEvents);
		bool								isDefaultLocationExist(const std::vector<Location>& locations);


		void								addClientToMap(int clientFd, int serverFd);
		
		bool								isListeningSocket(int fd);
		bool								isClientSocket(int fd);

		//void								closeIdleConnections();
		void								closeClientFd(int clientfFd);
		void								closeAllSockets(); 


		const	vServer*							findServerConfigByName(const std::vector<const vServer*>& subConfigs, std::string serverName) const;
		const	std::vector<const vServer*>			findServerConfigsByFd(int serverFd) const;
		const	Location*							findLocationBlockByUri(const vServer& serverConfig, const std::string& url) const;
		const	Location*							findDefaultLocationBlock(const std::vector<Location>& locations) const;



		class ServerManagerException : public std::exception {
			private:
				std::string	_message;
			public:
				ServerManagerException(const std::string& message);
				const char*	what() const noexcept override;
		};
};


#endif