/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 10:34:01 by amysiv            #+#    #+#             */
/*   Updated: 2025/08/19 11:06:19 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */




#include "vServer.hpp"




vServer::vServer() {
	_vServerIp = "0.0.0.0";
	_vServerPort = "8080";
	_vServerIpPort = "0.0.0.0:8080";
	_vServerNames = {"localhost"};
	_vServerRoot = "website/";
	_vServerIndex = {"index.html"};
	_vServerAutoIndex = false;
	_vServerClientMaxSize = 1024 * 1024 * 1024;
	_vServerErrorPages = {
		{400, "/errors/400.html"},
		{403, "/errors/403.html"},
		{404, "/errors/404.html"},
		{405, "/errors/405.html"},
		{413, "/errors/413.html"},
		{500, "/errors/500.html"},
		{502, "/errors/502.html"},
		{503, "/errors/503.html"},
	};
}




vServer::~vServer() {

}




//setters
void	vServer::setServerRoot(const std::string& path) {
	_vServerRoot = path;
}


void	vServer::setServerLocations(const std::map<std::string, Location>& loc) {
_vServerLocations = loc;
}


void	vServer::setServerIndex(const std::vector<std::string>& index) {
	_vServerIndex = index;
}


void	vServer::setServerAutoIndex(const int mode) {
	_vServerAutoIndex = mode;
}


void	vServer::setServerClientMaxSize(const uint64_t size) {
	_vServerClientMaxSize = size;
}


void	vServer::setServerErrorPages(const std::unordered_map<int, std::string>& pages) {
	_vServerErrorPages = pages;
}




//getters
bool									vServer::getServerAutoIndex(void) const {
	return (_vServerAutoIndex);
}


std::string								vServer::getServerIp(void) const {
	return (_vServerIp);
}


std::string								vServer::getServerPort(void) const {
	return (_vServerPort);
}


std::string								vServer::getServerIpPort(void) const {
	return (_vServerIpPort);
}


std::string								vServer::getServerRoot(void) const {
	return(_vServerRoot);
}


std::vector<std::string>				vServer::getServerIndex(void) const {
	return (_vServerIndex);
}


std::map<std::string, Location>&		vServer::getServerLocations(void) {
	return _vServerLocations;
}


const std::map<std::string, Location>&	vServer::getServerLocations(void) const {
	return _vServerLocations;
}


std::unordered_map<int, std::string>	vServer::getServerErrorPages( void ) const {
	return(_vServerErrorPages);
}


std::unordered_set<std::string>			vServer::getServerNames( void ) const {
	return (_vServerNames);
}


uint64_t								vServer::getServerClientMaxSize( void ) const {
	return(_vServerClientMaxSize);
}





//validators
void vServer::validateServerListen(const std::vector<std::string>& addressVector) {
	if (addressVector.size() != 1) {
		throw ParseConfig::ConfException("Invalid listen directive: expected one argument (e.g., IP:Port, Port, or IP).");
	}

	const std::string& input = addressVector.at(0);
	std::smatch 		matches;
	std::string 		ipStr;
	std::string 		portStr;

	// Patterns
	std::regex ipv4WithPort(R"(^(\d{1,3}(?:\.\d{1,3}){3}):([0-9]{1,5})$)");
	std::regex portOnly(R"(^([0-9]{1,5})$)");
	std::regex ipv4Only(R"(^(\d{1,3}(?:\.\d{1,3}){3})$)");
	std::regex ipv6WithPort(R"(^\[(.+)\]:([0-9]{1,5})$)");
	std::regex ipv6Only(R"(^\[(.+)\]$)");
	std::regex localhostPort(R"(^(localhost):([0-9]{1,5})$)");
	std::regex localhost(R"(^(localhost)$)");

	if (std::regex_match(input, matches, ipv4WithPort)) {
		ipStr = matches[1];
		portStr = matches[2];
	} else if (std::regex_match(input, matches, ipv4Only)) {
		ipStr = matches[1];
	} else if (std::regex_match(input, matches, portOnly)) {
		portStr = matches[1];
	} else if (std::regex_match(input, matches, ipv6WithPort)) {
		ipStr = matches[1];
		portStr = matches[2];
	} else if (std::regex_match(input, matches, ipv6Only)) {
		ipStr = matches[1];
	} else if (std::regex_match(input, matches, localhost)){
		ipStr = matches[1];
	} else if (std::regex_match(input, matches, localhostPort)){
		ipStr = matches[1];
		portStr = matches[2];
	}else {
		throw ParseConfig::ConfException("Listen directive format is invalid. Acceptable formats: IP:Port, IP, Port, [IPv6]:Port, [IPv6]");
	}
	if (!portStr.empty()) {

		int portInt;
		try {
			portInt = std::stoi(portStr);
		} catch (...) {
			throw ParseConfig::ConfException("Port is not a valid number.");
		}
		if (portInt < MIN_PORT_NUMB || portInt > MAX_PORT_NUMB) {
			throw ParseConfig::ConfException("Port number is out of valid range (1–65535).");
		}
	}

	if (!ipStr.empty())
		_vServerIp = ipStr;
	if (!portStr.empty())
		_vServerPort = portStr;

	std::string ipFormatted;

	if (ipStr.find(':') != std::string::npos) {
		ipFormatted = "[" + ipStr + "]";
	} else {
		ipFormatted = ipStr;
	}
	_vServerIpPort = ipFormatted + ":" + portStr;
}






void	vServer::validateServerNames(std::vector<std::string>& namesVector) {

	if (namesVector.size() > MAX_SERVER_NAME_NUMB) {
		throw ParseConfig::ConfException("Invalid server-name directive: max 2 names");
	}

	for (std::string& name : namesVector) {
			_vServerNames.clear();
			_vServerNames.insert(name);
	}
}


const std::string&	vServer::onlyOneArgumentCheck(const std::vector<std::string>& pathVector, std::string directiveName) {

	if (pathVector.size() != MAX_ARG) {

		throw ParseConfig::ConfException("Invalid " + directiveName + " directive: only one argument expected for this field");
	}
	return (pathVector.at(0));
}


bool	vServer::validateAutoIndexDirective(const std::vector<std::string>& flagVector) {


	if (flagVector.size() != MAX_ARG) {

		throw ParseConfig::ConfException("Invalid auto-index directive: expected single value 'on' <= or => 'off' ");
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


uint64_t	vServer::validateClientMaxSizeDirective(const std::vector<std::string>& sizeVector) {

	
	if (sizeVector.size() != MAX_ARG) {
		throw ParseConfig::ConfException("Invalid client_max_body_size directive: expected one argument.");
	}
	std::string suffix = "";
	
	for (size_t i = 0; i < sizeVector[0].size(); ++i) {
		if (!isdigit(sizeVector[0][i])) 
		{
			if ((sizeVector[0][i] == 'k' || sizeVector[0][i] == 'K' || sizeVector[0][i] == 'm' || sizeVector[0][i] == 'M' || sizeVector[0][i] == 'g' || sizeVector[0][i] == 'G') && suffix.empty()) {
				suffix = sizeVector[0].substr(i);
				break;
			}
			else {
				throw ParseConfig::ConfException("Invalid size format.");
			}
		}
	}
	uint64_t number;
	try {
		number = std::stoull(sizeVector[0]);
	}
	catch (const std::exception& e) {
		throw ParseConfig::ConfException("Invalid number in size: " + std::string(e.what()));
	}

	uint64_t mult = 1;
	if (suffix == "k" || suffix == "K") {
		mult = 1024ULL;
	} else if (suffix == "m" || suffix == "M") {
		mult = 1024ULL * 1024;
	} else if (suffix == "g" || suffix == "G") {
		mult = 1024ULL * 1024 * 1024;
	} else if (!suffix.empty()) {
		throw ParseConfig::ConfException("Invalid size suffix: " + suffix);
	}

	const uint64_t limit = static_cast<uint64_t>(MAX_CLIENT_BODY_SIZE) * 1024 * 1024 * 1024;

	if (number > UINT64_MAX / mult) {
		throw ParseConfig::ConfException("Size value too large, would overflow.");
	}

	uint64_t final_size = number * mult;

	if (final_size > limit) {
		throw ParseConfig::ConfException("client_max_body_size directive: Exceeded the limit of " + std::to_string(MAX_CLIENT_BODY_SIZE) + "G");
	}

	if (final_size == 0) {
		return ULLONG_MAX; // Special case: unlimited
	}
	return final_size;
}




std::unordered_map<int, std::string>	vServer::validateErrorPagesDirective(const std::vector<std::string>& errorPagesVector) {

	int										errorCode;
	std::unordered_map<int, std::string>	errorPagesMap;

	if (errorPagesVector.size() != MAX_ARG_ERROR_PAGE) {

		throw ParseConfig::ConfException("Invalid error_pages directive: expected error code and path");
	}

	try {
		errorCode =  stoi(errorPagesVector.at(0), nullptr, 10);
	}
	catch (const std::exception& e) {
		throw ParseConfig::ConfException("Invalid error code: must be a number");
	}
	if (errorCode < 400 || errorCode > 599)
		throw ParseConfig::ConfException("Invalid error code: " + std::to_string(errorCode));
		
	const	std::string& errorPagePath = errorPagesVector.at(1);
	if (errorPagePath.empty()) {

		throw ParseConfig::ConfException("Invalid error page path: must start with '/'");
	}

	errorPagesMap[errorCode] = errorPagePath;
	return	errorPagesMap;
}


bool	isNumber(std::string number) {

	for (char c : number) {
		if (!isdigit(c))
			return (false);
	}
	return (true);
}