#include "vServer.hpp" 
#include "ParseConfig.hpp" 

vServer::vServer() {

	_vServerIp = "0.0.0.0:80";
	_vServerNames = {"localhost"};
	_vServerRoot = "/myWebsite/pages";
	_vServerIndex = "index.html";
	_vServerAutoIndex = false;
	_vServerClientMaxSize = 1024 * 10;
}


Location::Location(const vServer& serv) {

	_locationPath = "/";
	_locationRoot= serv.getServerRoot();
	_locationIndex = serv.getServerIndex();
	_locationAutoIndex = serv.getServerAutoIndex();
	_locationClientMaxSize = serv.getServerClientMaxSize();
	_locationAllowedMethods = serv.getServerAllowedMethods();
	_locationErrorPages = serv.getServerErrorPages();
	
}

vServer::~vServer() {

}




void	vServer::setServerListen(const std::vector<std::string>& addressVector){

	std::regex	ipV4Pattern(R"((\d{1,3}(?:\.\d{1,3}){3}):(\d{1,5}))");
	std::smatch	matches;
	
	if (addressVector.size() != MAX_ARG) {

		throw std::runtime_error("Invalid listen directive: expected 'address:port' format");
	}

	const	std::string address = addressVector.at(0);
	
	if (std::regex_match(address, matches, ipV4Pattern)) {
		
		std::string	ipStr = matches[1];
		std::string	portStr = matches[2];
		int	portInt = std::stoi(portStr);
		if (portInt >= MIN_PORT_NUMB && portInt <= MAX_PORT_NUMB) {

			_vServerIp = ipStr;
			_vServerPort = portStr;
		}
		else
			throw ParseConfig::ConfException("Port is out of range");
	}
	else {

		std::cout << "Input does not match the IPv4:Port format.\n";
	}
}

void	vServer::setServerNames(std::vector<std::string>& namesVector) {

	if (namesVector.size() > MAX_SERVER_NAME_NUMB) {

		throw std::runtime_error("Invalid server-name directive: max 2 names");
	}

	std::regex namePattern(R"(^[a-zA-Z0-9-]+(\.[a-zA-Z0-9-]+)+$)");

	for (std::string& name : namesVector) {

		if (std::regex_match(name, namePattern))
			_vServerNames.push_back(name);
		else
			throw ParseConfig::ConfException("Input does not match 'example.com' format .\n");
	}
}

void	vServer::setServerRoot(const std::vector<std::string>& pathVec) {
	_vServerRoot = onlyOneArgumentCheck(pathVec, "root");
}

void	vServer::setServerIndex(const std::vector<std::string>& indexVec) {
	_vServerIndex = onlyOneArgumentCheck(indexVec, "index");
}

void	vServer::setServerAutoIndex(const std::vector<std::string>& flagVec) {
	_vServerAutoIndex = validateAutoIndexDirective(flagVec);
}

void	vServer::setServerClientMaxSize(const std::vector<std::string>& sizevec) {
	_vServerClientMaxSize = validateClientMaxSizeDirective(sizevec);
}

void	vServer::setServerAllowedMethods(const std::vector<std::string>& methods) {
	_vServerAllowedMethods = validateAllowedMethodsDirective(methods);
}

void	vServer::setServerErrorPages(const std::vector<std::string>& pages) {

	_vServerErrorPages = validateErrorPagesDirective(pages);
}


bool	vServer::getServerAutoIndex( void ) const {
	return (_vServerAutoIndex);
}

std::string	vServer::getServerIp( void ) const {
	return (_vServerIp);
}

std::string	vServer::getServerPort( void ) const {
	return (_vServerPort);
}


std::string								vServer::getServerRoot( void ) const {
	return(_vServerRoot);
}

std::string								vServer::getServerIndex( void ) const {
	return (_vServerIndex);
}

std::vector<Location>					vServer::getServerLocations( void ) const {
	return(_vServerLocations);
}

std::unordered_map<int, std::string>	vServer::getServerErrorPages( void ) const {
	return(_vServerErrorPages);
}

std::vector<std::string>				vServer::getServerNames( void ) const {
	return (_vServerNames);
}

unsigned								vServer::getServerClientMaxSize( void ) const {
	return(_vServerClientMaxSize);
}

std::vector<std::string>				vServer::getServerAllowedMethods( void ) const {
	return(_vServerAllowedMethods);
}


std::string	vServer::onlyOneArgumentCheck(const std::vector<std::string>& pathVector, std::string directiveName) {

	if (pathVector.size() != MAX_ARG) {

		throw std::runtime_error("Invalid " + directiveName + "directive: only one argument for this field");
	}
	return (pathVector.at(0));
}




bool	vServer::validateAutoIndexDirective(const std::vector<std::string>& flagVector) {


	if (flagVector.size() != MAX_ARG) {

		throw std::runtime_error("Invalid auto-index directive: expected single value 'on' <= or => 'off' ");
	}

	const std::string&	flag = flagVector.at(0);

	if (flag == "on") {
		return (true);
	}
	else if (flag == "off") {
		return (false);
	}
	else {

		throw ParseConfig::ConfException("Inncorect option: '" + flag + "' for auto-index field\n");
	}
}

unsigned	vServer::megaBytesToBits( int MB) {

	return (MB * 1024 *1024);
}

size_t	vServer::validateClientMaxSizeDirective(const std::vector<std::string>& sizeVector) {

	size_t	clientBodySizeMB;

	if (sizeVector.size() != MAX_ARG) {

		throw std::runtime_error("Invalid client-max-size directive: expected single value in M ");
	}

	const std::string&	clientBodySizeStr = sizeVector.at(0);
	try {

		clientBodySizeMB = std::stoi(clientBodySizeStr, nullptr, 10);
	}

	catch (const std::exception& e) {

		throw std::runtime_error("Invalid body size: must be a number");
	}

	if ( MIN_CLIENT_BODY_SIZE < clientBodySizeMB && clientBodySizeMB <= MAX_CLIENT_BODY_SIZE)
		return (vServer::megaBytesToBits(clientBodySizeMB));
	else
		throw ParseConfig::ConfException("Client-max-size: out of range: Range 1Mb <=> 20 MB");

}


 std::vector<std::string>	vServer::validateAllowedMethodsDirective(const std::vector<std::string>& methodsVector) {

	std::vector<std::string>		validMethods;
	std::set<std::string>			allowedMethodsSet = {"POST", "GET", "DELETE"};


	for (const std::string&	method : methodsVector) {
	
		if (allowedMethodsSet.count(method)){
			validMethods.push_back(method);
		}
		else
			std::cout<< "NOT ALLOWED: " + method <<"\n";
		}
		return (validMethods);
}


std::unordered_map<int, std::string>	vServer::validateErrorPagesDirective(const std::vector<std::string>& errorPagesVector) {

	std::set<int>							errorCodesSet {404, 403, 409, 500, 301};
	int										errorCode;
	std::unordered_map<int, std::string>	errorPagesMap;

	if (errorPagesVector.size() != MAX_ARG_ERROR_PAGE) {

		throw std::runtime_error("Invalid error_pages directive: expected error code and path");
	}

	try {

		errorCode =  stoi(errorPagesVector.at(0), nullptr, 10);
	}

	catch (const std::exception& e) {

		throw std::runtime_error("Invalid error code: must be a number");
	}
	
	if (!errorCodesSet .count(errorCode)) {
		throw std::runtime_error("Unsupported error code: " + std::to_string(errorCode));
	}

	const	std::string& errorPagePath = errorPagesVector.at(1);

	if (errorPagePath.empty() || errorPagePath.at(0) != '/') {

		throw std::runtime_error("Invalid error page path: must start with '/'");
	}

	errorPagesMap[errorCode] = errorPagePath;
	return	errorPagesMap;
}


void	Location::setLocationPath(const std::string& pathToCheck) {

	if (pathToCheck.empty() || pathToCheck.at(0) != '/')
		throw std::runtime_error("Location directive must be followed by a ' /path ' ");
	else	
		_locationPath = pathToCheck;
}