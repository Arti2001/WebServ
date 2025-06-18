#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "vServer.hpp"

class Location {
	private:
		std::string								_locationPath;
		std::string								_locationUploadPath;
		std::string								_locationRoot;
		std::string								_locationIndex;
		int										_locationAutoIndex;
		unsigned								_locationClientMaxSize;
		std::vector<std::string>				_locationAllowedMethods;
		std::pair<int, std::string>				_locationReturnPages;
		std::unordered_map<int, std::string>	_locationErrorPages;
	public:

		Location();
		Location(const vServer& serv);
		Location(const Location& other);
		const Location&	operator=(const Location& other);


	//getters
	const std::string&							getLocationPath(void) const;
	const std::string&							getLocationUploadPath(void) const;
	const std::string&							getLocationRoot(void) const;
	const std::string&							getLocationIndex(void) const;
	const int&									getLocationAutoIndex(void) const;
	const unsigned&								getLocationClientMaxSize(void) const;
	const std::vector<std::string>&				getLocationAllowedMethods(void) const; // better to use set, as it won't allow duplicate methods
	const std::pair<int, std::string>&			getLocationReturnPages(void) const;
	const std::unordered_map<int, std::string>&	getLocationErrorPages(void) const;
	// need to add cgi support here
	//Setters
	void									setLocationPath(const std::string&path);
	void									setLocationUploadPath(const std::string& path);
	void									setLocationRoot(const std::string& root);
	void									setLocationIndex(const std::string& index);
	void									setLocationAutoIndex(const int autoIndex);
	void									setLocationClientMaxSize(const unsigned maxSize);
	void									setLocationAllowedMethods(const std::vector<std::string>& methods);
	void									setLocationReturnPages(const std::pair<int, std::string>& returnPages);
	void									setLocationErrorPages(const std::unordered_map<int, std::string>& errorPages);

	static std::pair<int, std::string>	setLocationReturnPages(std::vector<std::string>& returnPageVector);
	// inside of the location block we can override pretty much everything from the server block, so we should be 
	// using same logic as in vServer class
	//validators
	void								validateAllowedMethodsDirective(const std::vector<std::string>& allowedMethodsVector);

};

#endif