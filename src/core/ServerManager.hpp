#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#include "parsingConfFile/ParseConfig.hpp"
#include "Server.hpp"
#include <fstream>


#define EPOLL_CAPACITY 20

class ServerManager {

	private:
		//std::vector<Server>				_servers;
		//const	std::vector<vServer>&	_serverSettings;
		std::ifstream					_configFileFd;
		int								_epollFd;
		std::vector<int>				_socketFds;
		std::vector<vServer>			_vServers;

	public:
		//constructors
		ServerManager(std::string& ConfigFileName, int epollSize);
		~ServerManager();

		//getters
		std::ifstream				getConfigFileFd( void ) const;
		
		int							getEpollFd( void ) const;
		std::vector<int>			getSocketFds( void ) const;
		std::vector<vServer>&		getVServers( void ) ;


		addrinfo*					getAddrList( size_t ServerCounter) ;
		
		//setter
		void						setSocketFds( size_t ServerCounter );


		//methods
		void	parsConfigFile(std::vector<vServer>& _vServers);
		int		bindSocket(addrinfo* addrList);
		bool	setNonBlocking(int fd);







		class ServerManagerException : public std::exception {
			private:
				std::string	_message;
			public:
				ServerManagerException(const std::string& message);
				const char*	what() const noexcept override;
		};
};

#endif