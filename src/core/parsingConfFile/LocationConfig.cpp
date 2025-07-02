#include "LocationConfig.hpp"

Location::Location() {
}

Location::Location(const vServer& serv) {

	_locationPath = "/";
	_locationUploadPath = serv.getServerRoot() + "uploads/";
	_locationReturnPages = {0, ""};
	_locationRoot= serv.getServerRoot();
	_locationIndex = serv.getServerIndex();
	_locationAutoIndex = serv.getServerAutoIndex();
	_locationClientMaxSize = serv.getServerClientMaxSize();
	_locationAllowedMethods = {"GET"};
	_locationErrorPages = serv.getServerErrorPages();
}

Location::Location(const Location& other) {
	*this = other;
}

const Location&	Location::operator=(const Location& other) {
	if (this != &other) {
		this->_locationPath = other._locationPath;
		this->_locationUploadPath = other._locationUploadPath;
		this->_locationRoot = other._locationRoot;
		this->_locationIndex = other._locationIndex;
		this->_locationAutoIndex = other._locationAutoIndex;
		this->_locationClientMaxSize = other._locationClientMaxSize;
		this->_locationAllowedMethods = other._locationAllowedMethods;
		this->_locationReturnPages = other._locationReturnPages;
		this->_locationErrorPages = other._locationErrorPages;
		this->_locationAllowedCgi = other._locationAllowedCgi;
	}
	return (*this);
}

//getters

const std::string& Location::getLocationPath() const {
	return _locationPath;
}

const std::string& Location::getLocationUploadPath() const {
	return _locationUploadPath;
}

const std::string& Location::getLocationRoot() const {
	return _locationRoot;
}

//const std::string& Location::getLocationIndex() const {
//	return _locationIndex;
//}

const std::vector<std::string>& Location::getLocationIndex() const {
	return _locationIndex;
}

const int& Location::getLocationAutoIndex() const {
	return _locationAutoIndex;
}

const unsigned& Location::getLocationClientMaxSize() const {
	return _locationClientMaxSize;
}

const std::unordered_set<std::string>& Location::getLocationAllowedMethods() const {
	return _locationAllowedMethods;
}

const std::map<std::string, std::string>& Location::getLocationAllowedCgi() const {
	return _locationAllowedCgi;
}

const std::pair<int, std::string>& Location::getLocationReturnPages() const {
	return _locationReturnPages;
}

const std::unordered_map<int, std::string>& Location::getLocationErrorPages() const {
	return _locationErrorPages;
}


//setters
void Location::setLocationPath(const std::string& path) {
	_locationPath = path;
}

void Location::setLocationUploadPath(const std::string& path) {
	_locationUploadPath = path;
}

void Location::setLocationRoot(const std::string& root) {
	_locationRoot = root;
}

//void Location::setLocationIndex(const std::string& index) {
//	_locationIndex = index;
//}

void Location::setLocationIndex(const std::vector<std::string>& index) {
	_locationIndex = index;
}

void Location::setLocationAutoIndex(const int autoIndex) {
	_locationAutoIndex = autoIndex;
}

void Location::setLocationClientMaxSize(const unsigned maxSize) {
	_locationClientMaxSize = maxSize;
}

void Location::setLocationAllowedMethods(const std::unordered_set<std::string>& methods) {
	_locationAllowedMethods = methods;
}

void Location::setLocationReturnPages(const std::pair<int, std::string>& returnPages) {
	_locationReturnPages = returnPages;
}

void Location::setLocationErrorPages(const std::unordered_map<int, std::string>& errorPages) {
	_locationErrorPages = errorPages;
}


std::pair<int, std::string> Location::setLocationReturnPages(std::vector<std::string>& redirVector) {

	int							returnCode = -1;
	std::string					path = "";
	std::pair<int, std::string>	codePathPair;

	if (redirVector.size() > MAX_ARG_ERROR_PAGE) { // same as in cgi directive
		throw ParseConfig::ConfException("Invalid return directive: too many arguments!");
	}

	if (isNumber(redirVector.at(0)) && redirVector.size() == 2) {
		try {
			returnCode = stoi(redirVector.at(0), nullptr, 10);
			path = redirVector.at(1);
		}
		catch (const std::exception& e) {
			throw ParseConfig::ConfException("Invalid return code: stoi() failed.");
		}
	}

	else if (redirVector.size() == 1) {
		if (isNumber(redirVector.at(0))) {
			try {
				returnCode = stoi(redirVector.at(0), nullptr, 10);
			}
			catch (const std::exception& e) {
				throw ParseConfig::ConfException("Invalid return code: stoi() failed.");
			}
		}
		else {
			path = redirVector.at(0);
		}
	}
	else {
		throw ParseConfig::ConfException("Bad syntax for return field");
	}
	
	if (returnCode == -1)
		throw ParseConfig::ConfException("Return directive must include a status code.");

	return(std::make_pair(returnCode, path));
}




void	Location::validateAllowedMethodsDirective(const std::vector<std::string>& methodsVector) {

	_locationAllowedMethods.clear();
	const std::set<std::string>	allowedMethodsSet = {"POST", "GET", "DELETE"};

	for (const std::string& method : methodsVector) {
		
		if (allowedMethodsSet.count(method)) {
			
			if (!_locationAllowedMethods.count(method))
				_locationAllowedMethods.insert(method);
			else
				throw ParseConfig::ConfException("Duplicated method: " + method);
		}
		else
			throw ParseConfig::ConfException("Invalid method: " + method);
	}
}

void Location::validateAllowedCgiDirective(const std::vector<std::string>& cgiVector) {

	if (cgiVector.size() != MAX_ARG_ERROR_PAGE) { // better to use a number instead of a macro that has an ambiguous name
		throw ParseConfig::ConfException("Invalid allowed_cgi directive: Invalid amount of arguments.");
	}
	const std::string& ext = cgiVector.at(0);
	const std::string& path = cgiVector.at(1);


	if (_locationAllowedCgi.find(ext) == _locationAllowedCgi.end()) 
	{
		_locationAllowedCgi[ext] = path;
	}
	else
		throw ParseConfig::ConfException("Duplicated " + ext + " extension.");
}