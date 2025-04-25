#include "vServer.hpp" 
#include "ParseConfig.hpp" 

vServer::vServer() {

	_ip = "0.0.0.0:80";
	_serverNames = {"localhost"};
	_root = "/myWebsite/pages";
	_index = "index.html";
	_auto_index = false;
	_clientMaxSize = 1024 ^ 4;
}


Location::Location(const vServer& serv) {

	_path = "/";
	_root = serv.getRoot();
	_index = serv.getIndex();
	_auto_index = serv.getAutoIndex();
	_clientMaxSize = serv.getClientMaxSize();
	_allowedMethods = serv.getAllowedMethods();
	_errorPages = serv.getErrorPages();
	
}

vServer::~vServer() {
    // your destructor code (even if empty)
}




void	vServer::setListen(const std::vector<std::string>& addrVec){

	std::regex	ipV4pattern(R"((\d{1,3}(?:\.\d{1,3}){3}):(\d{1,5}))");
	std::smatch	matches;
	std::string	ip;
	std::string	portStr;
	int 		port;
	
	if (addrVec.size() != 1) {
		throw std::runtime_error("Invalid listen directive: expected 'address:port' format");
	}

	const	std::string addr = addrVec[0];
	
	if (std::regex_match(addr, matches, ipV4pattern)) {

		ip = matches[1];
		portStr = matches[2];
	
		port = std::stoi(portStr);

		if (port >= 1 && port <= 65535) {
			_ip = ip.c_str();
			_port = portStr.c_str();
		}
		else
			throw ParseConfig::ConfException("Port is out of range");
	}
	else {
		std::cout << "Input does not match the IPv4:Port format.\n";
	}
}

void	vServer::setServerName(std::vector<std::string>& namesVec) {

	if (namesVec.size() > 2) {
		throw std::runtime_error("Invalid server-name directive: max 2 names");
	}

	std::regex namePattern(R"(^[a-zA-Z0-9-]+(\.[a-zA-Z0-9-]+)+$)");

	for (std::string& name : namesVec) {

		if (std::regex_match(name, namePattern))
			_serverNames.push_back(name);
		else
			throw ParseConfig::ConfException("Input does not match 'example.com' format .\n");
	}
}

void	vServer::setRoot(const std::vector<std::string>& pathVec) {
	_root = onlyOneCheck(pathVec, "root");
}

void	vServer::setIndex(const std::vector<std::string>& indexVec) {
	_index = onlyOneCheck(indexVec, "index");
}

void	vServer::setAutoIndex(const std::vector<std::string>& flagVec) {
	_auto_index = validAutoIndex(flagVec);
}

void	vServer::setClientMaxSize(const std::vector<std::string>& sizevec) {
	_clientMaxSize = validClientMaxSize(sizevec);
}

void	vServer::setAllowedMethods(const std::vector<std::string>& methods) {
	_allowedMethods = validAllowedMethods(methods);
}

void	vServer::setErrorPages(const std::vector<std::string>& pages) {

	_errorPages = validErrorPages(pages);
}


bool	vServer::getAutoIndex( void ) const {
	return (_auto_index);
}

const char*	vServer::getIp( void ) const {
	return (_ip);
}

const char*	vServer::getPort( void ) const {
	return (_port);
}


std::string								vServer::getRoot( void ) const {
	return(_root);
}

std::string								vServer::getIndex( void ) const {
	return (_index);
}

std::vector<Location>					vServer::getLocations( void ) const {
	return(_locations);
}

std::unordered_map<int, std::string>	vServer::getErrorPages( void ) const {
	return(_errorPages);
}

std::vector<std::string>				vServer::getServerNames( void ) const {
	return (_serverNames);
}

unsigned								vServer::getClientMaxSize( void ) const {
	return(_clientMaxSize);
}

std::vector<std::string>				vServer::getAllowedMethods( void ) const {
	return(_allowedMethods);
}


std::string	vServer::onlyOneCheck(const std::vector<std::string>& pathVec, std::string dir) {

	if (pathVec.size() != 1) {
		throw std::runtime_error("Invalid " + dir + "directive: only one argument for this field");
	}
	return (pathVec[0]);
}

bool	vServer::validAutoIndex(const std::vector<std::string>& flagVec) {


	if (flagVec.size() != 1) {
		throw std::runtime_error("Invalid auto-index directive: expected single value 'on' <= or => 'off' ");
	}

	const std::string&	path = flagVec[0];

	if (path == "on") {
		return (true);
	}
	else if (path == "off") {
		return (false);
	}
	else {
		throw ParseConfig::ConfException("Inncorect option: '" + path + "' for auto-index field\n");
	}
}

size_t	vServer::validClientMaxSize(const std::vector<std::string>& sizevec) {

	size_t	size;

	if (sizevec.size() != 1) {
		throw std::runtime_error("Invalid client-max-size directive: expected single value in M ");
	}
	const std::string&	path = sizevec[0];
	try {
		size = std::stoi(path, nullptr, 10);
	}
	catch (const std::exception& e) {
		throw std::runtime_error("Invalid body size: must be a number");
	}
	if ( 0 < size && size <= 20)
		return (size  * 1024 * 1024);
	else
		throw ParseConfig::ConfException("Client-max-size: out of range: Range 1Mb <=> 20 Mb");

}


 std::vector<std::string>	vServer::validAllowedMethods(const std::vector<std::string>& methods) {

	std::vector<std::string>		validMethods;
	std::set<std::string>			allowed = {"POST", "GET", "DELETE"};


	for (const std::string&	method : methods) {

	
		if (allowed.count(method)){
			validMethods.push_back(method);
		}
		else
			std::cout<< "NOT ALLOWED: " + method <<"\n";
		}
		return (validMethods);
}

std::unordered_map<int, std::string>	vServer::validErrorPages(const std::vector<std::string>& pages) {

	std::set<int>							setErrors {404, 403, 409, 500, 301};
	int										errorCode;
	std::unordered_map<int, std::string>	errorMap;

	if (pages.size() < 2) {
		throw std::runtime_error("Invalid error_pages directive: expected error code and path");
	}

	try {

		errorCode =  stoi(pages[0], nullptr, 10);
	}
	catch (const std::exception& e) {
		throw std::runtime_error("Invalid error code: must be a number");
	}
	if (!setErrors.count(errorCode)) {
		throw std::runtime_error("Unsupported error code: " + std::to_string(errorCode));
	}

	const	std::string& path = pages[1];

	if (path.empty() || path.at(0) != '/')
		throw std::runtime_error("Invalid error page path: must start with '/'");

	
	errorMap[errorCode] = pages[1];
	return errorMap;
}


void	Location::setPath(const std::string& path) {

	if (path.empty() || path.at(0) != '/')
		throw std::runtime_error("Location directive must be followed by a ' /path ' ");
	else	
		_path = path;
}