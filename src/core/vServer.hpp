#ifndef VSERVER_HPP
#define VSERVER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>
#include <set>
#include <exception>
//
// ─── LOCATION STRUCT ────────────────────────────────────────────────────────────
//
struct Location {
	Location(const vServer& serv);

	// Configuration fields
	std::string								_path;
	std::string								_root;
	std::string								_index;
	bool									_auto_index;
	unsigned								_clientMaxSize;
	std::vector<std::string>				_allowedMethods;
	std::unordered_map<int, std::string>	_errorPages;

	void	getPath(const std::string& path);

};

//
// ─── VIRTUAL SERVER CLASS ───────────────────────────────────────────────────────
//
class vServer {
	private:
		const char *							_ip;
		const char *							_port;
		std::vector<std::string>				_serverNames;
		std::vector<Location>					_locations;
		std::string								_root;
		std::string								_index;
		bool									_auto_index;
		unsigned								_clientMaxSize;
		std::vector<std::string>				_allowedMethods;
		std::unordered_map<int, std::string>	_errorPages;

	public:
		vServer();
		~vServer();
	//Getters

	
	bool										getAutoIndex( void ) const;
	const char*									getIp( void ) const;
	const char*									getPort( void ) const;
	unsigned									getClientMaxSize( void ) const;
	std::string									getRoot( void ) const;
	std::string									getIndex( void ) const;
	std::vector<Location>						getLocations( void) const;
	std::vector<std::string>					getServerNames( void ) const;
	std::vector<std::string>					getAllowedMethods( void ) const;
	std::unordered_map<int, std::string>		getErrorPages( void ) const;


		

	// Setters
	void	setListen(const std::vector<std::string>& addrVec);
	void	setServerName(std::vector<std::string>& names);
	void	addLocation(const Location& loc);
	void	setRoot(const std::vector<std::string>& pathVec);
	void	setIndex(const std::vector<std::string>& indexVec);
	void	setAutoIndex(const std::vector<std::string>& flagVec);
	void	setClientMaxSize(const std::vector<std::string>& sizeVec);
	void	setAllowedMethods(const std::vector<std::string>& methodsVec);
	void	setErrorPages(const std::vector<std::string>& pagesVec);


	

	//validators
};
static	bool															validAutoIndex(const std::vector<std::string>& flagVec);
static	size_t															validClientMaxSize(const std::vector<std::string>& sizevec);
static	std::unordered_map<int, std::string>							validErrorPages(const std::vector<std::string>& pages);
static	std::vector<std::string>										validAllowedMethods(const std::vector<std::string>& methods);
static	std::string														onlyOneCheck(const std::vector<std::string>& pathVec, std::string dir);

#endif