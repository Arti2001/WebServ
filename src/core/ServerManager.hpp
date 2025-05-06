#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#include "parsing/ParseConfig.hpp"
#include "Server.hpp"
#include <fstream>

class ServerManager {

	private:
		//std::vector<Server>				_servers;
		//const	std::vector<vServer>&	_serverSettings;
		std::ifstream					_configFileFd;

	public:
		ServerManager(std::string& ConfigFileName);
		~ServerManager();

		//getters
		std::ifstream&	getConfigFileFd( void );

		std::vector<vServer>	parsConfigFile();
		




		class ServerManagerException : public std::exception {
			private:
				std::string	_message;
			public:
				ServerManagerException(const std::string& message);
				const char*	what() const noexcept override;
		};
};

#endif