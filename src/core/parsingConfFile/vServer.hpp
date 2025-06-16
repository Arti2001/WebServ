#ifndef vServer_HPP
#define vServer_HPP

#include <string>
#include <vector>
#include <unordered_map>
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

#define MAX_CLIENT_BODY_SIZE 20
#define MIN_CLIENT_BODY_SIZE 1

#define MAX_ARG_ERROR_PAGE	2

class vServer;

// it is better to use Location as a class instead of a struct, so that we can encapsulate the logic and data together.
// it has it is own methods and can be extended in the future if needed.
// I will include my own implementation of the Location class in the respective files for your reference.
//class Location {
//	private:
//		std::string								_locationPath;
//		std::string								_locationRoot;
//		std::string								_locationIndex;
//		bool									_locationAutoIndex;
//		unsigned								_locationClientMaxSize;
//		std::vector<std::string>				_locationAllowedMethods;
//		std::pair<int, std::string>				_locationReturnPages;
//		std::unordered_map<int, std::string>	_locationErrorPages;
//	public:

//		Location();
//		Location(const vServer& serv);

	//static std::string					setLocationPath(std::string& path);
	//static std::pair<int, std::string>	setLocationReturnPages(std::vector<std::string>& returnPageVector);
	// inside of the location block we can override pretty much everything from the server block, so we should be 
	// using same logic as in vServer class
//};

class vServer {
	private:
		std::string								_vServerIp;
		std::string								_vServerPort;
		std::string								_vServerIpPort;
		std::vector<std::string>				_vServerNames;
		std::vector<Location>					_vServerLocations;
		std::string								_vServerRoot;
		std::string								_vServerIndex;
		bool									_vServerAutoIndex;
		unsigned								_vServerClientMaxSize;
		std::unordered_map<int, std::string>	_vServerErrorPages;

	public:
		vServer();
		~vServer();

	//Getters
	bool										getServerAutoIndex( void ) const;
	std::string									getServerIp( void ) const;
	std::string									getServerPort( void ) const;
	std::string									getServerIpPort( void ) const;
	unsigned									getServerClientMaxSize( void ) const;
	std::string									getServerRoot( void ) const;
	std::vector<std::string>					getServerNames( void ) const;
	std::string									getServerIndex( void ) const;
	
	// don't use same names for getters that have different return types, it is confusing
	std::vector<Location>&						getServerLocations(); // allows writing
	const std::vector<Location>&				getServerLocations() const; // allows reading
	// allowed methods should be a location scope directive
	std::unordered_map<int, std::string>		getServerErrorPages( void ) const;


	// Setters
	//void	setServerListen(const std::vector<std::string>& addressVector);
	//void	setServerNames(std::vector<std::string>& names);
	//void	setServerLocations(const Location& loc);
	//void	setServerRoot(const std::vector<std::string>& pathVector);
	//void	setServerIndex(const std::vector<std::string>& indexVector);
	//void	setServerAutoIndex(const std::vector<std::string>& flagVector);
	//void	setServerClientMaxSize(const std::vector<std::string>& sizeVector);
	//void	setServerErrorPages(const std::vector<std::string>& pagesVector);
	
	void	setServerLocations(const Location& loc);
	void	setServerRoot(const std::string& path);
	void	setServerIndex(const std::string& index);
	void	setServerAutoIndex(const int mode);
	void	setServerClientMaxSize(const unsigned size);
	void	setServerErrorPages(const std::unordered_map<int, std::string>& pages);
	
	
	
	
	//validators
	void																	validateServerListen(const std::vector<std::string>& addressVector);
	void																	validateServerNames(std::vector<std::string>& names);

	//common validators for vServer and Location calss	
	static	bool															validateAutoIndexDirective(const std::vector<std::string>& flagVector);
	static	size_t															validateClientMaxSizeDirective(const std::vector<std::string>& sizeVector);
	static	std::unordered_map<int, std::string>							validateErrorPagesDirective(const std::vector<std::string>& errorPagesVector);
	//static	std::vector<std::string>										validateAllowedMethodsDirective(const std::vector<std::string>& allowedMethodsVector);
	static const	std::string&													onlyOneArgumentCheck(const std::vector<std::string>& pathVector, std::string directiveName);

	//methods
	static	unsigned				megaBytesToBits(int	mB);
};

bool	isNumber(std::string number);

#endif