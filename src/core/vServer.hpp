#ifndef VSERVER_HPP
#define VSERVER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>
#include <set>
#include <exception>


#define MAX_PORT_NUMB 65535
#define MIN_PORT_NUMB 1

#define MAX_SERVER_NAME_NUMB 2
#define MAX_ARG	1

#define MAX_CLIENT_BODY_SIZE 20
#define MIN_CLIENT_BODY_SIZE 1

#define MAX_ARG_ERROR_PAGE	2

class vServer;

struct Location {
	Location();
	Location(const vServer& serv);

	// Configuration fields
	std::string								_locationPath;
	std::string								_locationRoot;
	std::string								_locationIndex;
	bool									_locationAutoIndex;
	unsigned								_locationClientMaxSize;
	std::vector<std::string>				_locationAllowedMethods;
	std::string								_locationReturnPages;
	std::unordered_map<int, std::string>	_locationErrorPages;

	static std::string	setLocationPath(std::string& path);

};

class vServer {
	private:
		std::string								_vServerIp;
		std::string								_vServerPort;
		std::vector<std::string>				_vServerNames;
		std::vector<Location>					_vServerLocations;
		std::string								_vServerRoot;
		std::string								_vServerIndex;
		bool									_vServerAutoIndex;
		unsigned								_vServerClientMaxSize;
		std::vector<std::string>				_vServerAllowedMethods;
		std::unordered_map<int, std::string>	_vServerErrorPages;

	public:
		vServer();
		~vServer();

	//Getters
	bool										getServerAutoIndex( void ) const;
	std::string									getServerIp( void ) const;
	std::string									getServerPort( void ) const;
	unsigned									getServerClientMaxSize( void ) const;
	std::string									getServerRoot( void ) const;
	std::vector<std::string>					getServerNames( void ) const;
	std::string									getServerIndex( void ) const;
	std::vector<Location>&						getServerLocations(); // allows writing
	const std::vector<Location>&				getServerLocations() const; // allows reading	std::vector<std::string>					getServerNames( void ) const;
	std::vector<std::string>					getServerAllowedMethods( void ) const;
	std::unordered_map<int, std::string>		getServerErrorPages( void ) const;


	// Setters
	void	setServerListen(const std::vector<std::string>& addressVector);
	void	setServerNames(std::vector<std::string>& names);
	void	setServerLocations(const Location& loc);
	void	setServerRoot(const std::vector<std::string>& pathVector);
	void	setServerIndex(const std::vector<std::string>& indexVector);
	void	setServerAutoIndex(const std::vector<std::string>& flagVector);
	void	setServerClientMaxSize(const std::vector<std::string>& sizeVector);
	void	setServerAllowedMethods(const std::vector<std::string>& methodsVector);
	void	setServerErrorPages(const std::vector<std::string>& pagesVector);


	
	
	//validators
	static	bool															validateAutoIndexDirective(const std::vector<std::string>& flagVector);
	//std::string																validateReturnDirective(const std::vector<std::string>& returnPagesVector);
	static	size_t															validateClientMaxSizeDirective(const std::vector<std::string>& sizeVector);
	static	std::unordered_map<int, std::string>							validateErrorPagesDirective(const std::vector<std::string>& errorPagesVector);
	static	std::vector<std::string>										validateAllowedMethodsDirective(const std::vector<std::string>& allowedMethodsVector);
	static	std::string														onlyOneArgumentCheck(const std::vector<std::string>& pathVector, std::string directiveName);
	

	//methods
	static	unsigned				megaBytesToBits(int	mB);
};

#endif