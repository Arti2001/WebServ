#ifndef vServer_HPP
#define vServer_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <regex>
#include <set>
#include <exception>

#include "ParseConfig.hpp" 
#include "LocationConfig.hpp"


#define MAX_PORT_NUMB 65535
#define MIN_PORT_NUMB 1
#define MAX_SERVER_NAME_NUMB 2
#define MAX_ARG	1
#define MAX_CLIENT_BODY_SIZE 10
#define MIN_CLIENT_BODY_SIZE 1
#define MAX_ARG_ERROR_PAGE	2


class vServer {
	private:
		std::string								_vServerIp;
		std::string								_vServerPort;
		std::string								_vServerIpPort;
		std::vector<std::string>				_vServerNames;
		std::map<std::string, Location>			_vServerLocations;
		std::string								_vServerRoot;
		//std::string								_vServerIndex;
		std::vector<std::string>				_vServerIndex;
		bool									_vServerAutoIndex;
		uint64_t								_vServerClientMaxSize;
		std::unordered_map<int, std::string>	_vServerErrorPages;

	public:
		vServer();
		~vServer();

	//Getters
	bool										getServerAutoIndex( void ) const;
	std::string									getServerIp( void ) const;
	std::string									getServerPort( void ) const;
	std::string									getServerIpPort( void ) const;
	uint64_t									getServerClientMaxSize( void ) const;
	std::string									getServerRoot( void ) const;
	std::vector<std::string>					getServerNames( void ) const;
	//std::string									getServerIndex( void ) const;
	std::vector<std::string>									getServerIndex( void ) const;
	

	std::map<std::string, Location>	&			getServerLocations(); // allows writing
	const std::map<std::string, Location>&		getServerLocations() const; // allows reading
	std::unordered_map<int, std::string>		getServerErrorPages( void ) const;


	// Setters

	void	setServerLocations(const Location& loc);
	void	setServerRoot(const std::string& path);
	//void	setServerIndex(const std::string& index);
	void	setServerIndex(const std::vector<std::string>& index);
	void	setServerAutoIndex(const int mode);
	void	setServerClientMaxSize(const uint64_t size);
	void	setServerErrorPages(const std::unordered_map<int, std::string>& pages);
	
	
	
	
	//validators
	void																	validateServerListen(const std::vector<std::string>& addressVector);
	void																	validateServerNames(std::vector<std::string>& names);

	//common validators for vServer and Location calss	
	static	bool															validateAutoIndexDirective(const std::vector<std::string>& flagVector);
	static	uint64_t														validateClientMaxSizeDirective(const std::vector<std::string>& sizeVector);
	static	std::unordered_map<int, std::string>							validateErrorPagesDirective(const std::vector<std::string>& errorPagesVector);
	static const	std::string&											onlyOneArgumentCheck(const std::vector<std::string>& pathVector, std::string directiveName);
};

bool	isNumber(std::string number);

#endif