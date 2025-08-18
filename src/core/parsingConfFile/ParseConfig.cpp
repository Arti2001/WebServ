/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 13:09:04 by vshkonda          #+#    #+#             */
/*   Updated: 2025/08/18 18:22:36 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseConfig.hpp"


ParseConfig::ParseConfig() : depth(0), currToken(0) {

	_keywords[";"] = SEMICOLON;
	_keywords["{"] = OPENED_BRACE;
	_keywords["}"] = CLOSED_BRACE;


	_keywords["root"] = ROOT_DIR;
	_keywords["index"] = INDEX_DIR;
	_keywords["listen"] = LISTEN_DIR;
	_keywords["return"] = RETURN_DIR;
	_keywords["server"] = SERVER_BLOCK;
	_keywords["location"] = LOCATION_BLOCK;
	_keywords["allowed_cgi"] = ALLOWED_CGI;
	_keywords["autoindex"] = AUTO_INDEX_DIR;
	_keywords["upload_path"] = UPLOAD_PATH;
	_keywords["server_name"] = SERVER_NAME_DIR;
	_keywords["error_page"] = ERROR_PAGE_DIR;
	_keywords["allowed_methods"] = ALLOWED_METHODS;
	_keywords["client_max_body_size"] = BODY_MAX_SIZE;
}

ParseConfig::ConfException::ConfException(const std::string& msg) : _message(msg) {

}


const char*	ParseConfig::ConfException::what() const noexcept {
	return (_message.c_str());
}



Token::Token(size_t	lineNumber, std::string word, TokenType tokenType) {

	this->line_number = lineNumber;
	this->lexem = word;
	this->type = tokenType;
}



ParseConfig::~ParseConfig() {
}


bool	ParseConfig::isTokenDirective(TokenType type) const {
	return (type == LISTEN_DIR || type == ROOT_DIR || 
			type == INDEX_DIR || type == SERVER_NAME_DIR || 
			type == ERROR_PAGE_DIR || type == AUTO_INDEX_DIR ||
			type == BODY_MAX_SIZE || type == ALLOWED_METHODS ||
			type == RETURN_DIR || type == UPLOAD_PATH || type == ALLOWED_CGI);
}


bool	ParseConfig::validBrace() {

	int	lvl = 0;

	for (size_t i = 0; i < _tokens.size(); i++) {

		if (_tokens[i].lexem == "{") {
			lvl++;
		}
		else if(_tokens[i].lexem == "}") {
			lvl--;
		}
	}

	if(lvl == 0) {
		return (true);
	}
	return (false);
}
const vServer	ParseConfig::createDefaultConfig() {

	vServer	defaultConfig;
	Location location (defaultConfig);

	defaultConfig.getServerLocations().emplace("/", location);

	return(defaultConfig);
}

/*
	This method parses and validates tokens from a config file
	@param reference to a vector of virtual servers
*/
void	ParseConfig::parseConfigFileTokens(std::vector<vServer>& _vServers) {

	if (!validBrace()) //check for  enclosed braces
		throw ConfException("Floating closing brace in the configuration file");
	for (; currToken < _tokens.size();currToken++) {

		if (_tokens[currToken].type == SERVER_BLOCK) { //we encountered a keyword server
			if (_tokens[currToken + 1].type != OPENED_BRACE) { // if server block is not folowed by a {  we throw an exception
				throw ConfException("Expected ' { ' after ' server '");
			}

			vServer	vserv; // create an instance of a server
			depth = LEVEL; // I neeed it to understand in what type of block I am currently at. If depth != 0  means I am in location block. Helps to track the depth of nested structure basically
			currToken += 2; // this variable I will increase when I pass a keyword, here by 2 because `server`, `{` are two keywords
			parsevServerBlock(vserv);//here the whole parsing magic happends
			if (vserv.getServerLocations().find("/") == vserv.getServerLocations().end()) //this is a map  path = Location instance, if config does not have a default location block we basically create one, and  add it to the vector of locations for current server
				vserv.getServerLocations().emplace("/", Location(vserv));
			_vServers.push_back(vserv); // here I add  a virtual server into the vector of  vServers
			seenDirectives.clear();//clearing a set of seen directives
		}
		else {
			throw ConfException("Alien object is detected at the line: " + std::to_string(_tokens[currToken].line_number)); // check for alien object before the server keyword
		}
	}
	if (_vServers.empty()) // we create a default instance of a vServer with default values, that are in the object constructor.
		_vServers.push_back(createDefaultConfig());

	std::cout << _vServers; // an overload, shows directives and their values.
}


std::ostream& operator<<(std::ostream& os, const std::vector<vServer>& servers) {
	for (size_t i = 0; i < servers.size(); ++i) {
		os << "\n\n==================== Server Block " << i << " ====================\n";
		os << servers[i];
	}
	return os;
}


std::ostream& operator<<(std::ostream& os, const vServer& server) {
	os << "Server IP:                    " << server.getServerIp() << "\n";
	os << "Server Port:                  " << server.getServerPort() << "\n";

	os << "Server Names:                 ";
	for (const std::string& name : server.getServerNames())
		os << name + " ";
	os<< "\n";

	os << "Client Max Body Size:         " << server.getServerClientMaxSize() << "\n";
	os << "Root:                         " << server.getServerRoot() << "\n";
	os << "Index:                        ";
	for (const std::string& index : server.getServerIndex())
		os<< index << " ";
	os<< "\n";
	os << "AutoIndex:                    " << server.getServerAutoIndex() << "\n";

	os << "  Error Pages:\n";
	const std::unordered_map<int, std::string>& errorPages = server.getServerErrorPages();
	for (std::unordered_map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		os << "    " << it->first << ": " << it->second << "\n";

	os << "------------------ Locations ------------------\n";
	const std::map<std::string, Location>& locs = server.getServerLocations();
	size_t i = 0;
	for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it, ++i) {
		const std::string& path = it->first;
		const Location& loc = it->second;

		os << "Location [" << i << "]\n";

		os << "  Path:           " << path << "\n";
		os << "  Root:           " << loc.getLocationRoot() << "\n";
		os << "  Index:          ";
		for (const std::string& index : loc.getLocationIndex())
			os<< index << " ";
		os<< "\n";
		os << "  AutoIndex:      " << loc.getLocationAutoIndex() << "\n";
		os << "  UploadPath:     " << loc.getLocationUploadPath() << "\n";
		os << "  Max Body Size:  " << loc.getLocationClientMaxSize() << "\n";
		os << "  Return:         ";
		const std::pair<int, std::string>& returnPages = loc.getLocationReturnPages();
		os<< returnPages.first << "   " << returnPages.second << "\n";
		os << "  Allowed Methods:";
		for (auto itSet = loc.getLocationAllowedMethods().begin(); itSet != loc.getLocationAllowedMethods().end(); ++itSet)
			os << " " << *itSet;
		//for (size_t j = 0; j < loc.getLocationAllowedMethods().size(); ++j)
		//	os << " " << loc.getLocationAllowedMethods().begin();
		os << "\n";
		os << "  Error Pages:";
		const std::unordered_map<int, std::string>& errorPages = loc.getLocationErrorPages();
		for (std::unordered_map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
			os << "    " << it->first << ": " << it->second << "\n";

		os << "  Allowed CGI:\n";
		const std::map<std::string, std::string>& allowedCgi = loc.getLocationAllowedCgi();
		for (std::map<std::string, std::string>::const_iterator it = allowedCgi.begin(); it != allowedCgi.end(); ++it)
			os << "    " << it->first << ": " << it->second << "\n" << "\n";


	}

	os << "=======================================================\n";
	return os;
}

/*
	This method parses a server block.
	@param reference to single virtual server object
*/
void	ParseConfig::parsevServerBlock(vServer& serv) {

	while (depth > 0) { //here depth is 1, we are in the server block

		if (_tokens[currToken].type == CLOSED_BRACE) {
			depth--;
			continue;
		}
		else if (isTokenDirective(_tokens[currToken].type)) {
			validateServerBlockDirectives(serv);
		}
		else if (_tokens[currToken].type == LOCATION_BLOCK) {
			depth += LEVEL;
			validateLocationBlockDirectives(serv);
			depth -= LEVEL;
		}
		else{
			std::cout<< "Trash: " +_tokens[currToken].lexem<< "\n";
			if (_tokens[currToken].type != COMMENT) 
				throw ConfException("Alien object is detected at the line: " + std::to_string(_tokens[currToken].line_number));
		}
		currToken++;
	}
}


std::string	ParseConfig::findLocationPath() {
	currToken++;//move to the path
	if (_tokens[currToken].lexem.at(0) != '/')
		throw ConfException(std::to_string(_tokens[currToken].line_number) + ":: Location directive must be followed by a '/path'");
	else if (_tokens[currToken + 1].type != OPENED_BRACE)
		throw ConfException(std::to_string(_tokens[currToken].line_number) + " :: No open brace.");

	std::string	path = _tokens[currToken].lexem;
	currToken += 2;//move to a directive
	return (path);
}

bool ParseConfig::noRepeatDirective(TokenType type) const {
	return (type == LISTEN_DIR || type == SERVER_NAME_DIR);
}

void	ParseConfig::isSeenDirective(Token directive) {

	TokenType type = directive.type;

	if (noRepeatDirective(type)) {
		if (!seenDirectives.count(type)) 
			seenDirectives.insert(type);
		else
			throw ConfException("Duplicated " + directive.lexem + " directive");
	}
}


void	ParseConfig::validateLocationBlockDirectives(vServer& vServer) {

Location	loc(vServer);

std::string locationPath = findLocationPath();
loc.setLocationPath(locationPath);
for (; _tokens[currToken].type != CLOSED_BRACE; currToken++) {

	if (_tokens[currToken].type == COMMENT) {
		std::cout << "FOUND COMENT : " + _tokens[currToken].lexem<<"\n";
		continue;
	}
	std::pair<Token, std::vector<std::string>> pair = makeKeyValuePair();

	switch (pair.first.type) {
		case ROOT_DIR:
			loc.setLocationRoot(vServer::onlyOneArgumentCheck(pair.second, "root"));
		break;

		case INDEX_DIR:
			loc.setLocationIndex(pair.second);
		break;

		case UPLOAD_PATH:
			loc.setLocationUploadPath(vServer::onlyOneArgumentCheck(pair.second, "upload_path"));
		break;

		case AUTO_INDEX_DIR:
			loc.setLocationAutoIndex(vServer::validateAutoIndexDirective(pair.second));
		break;

		case BODY_MAX_SIZE:
			loc.setLocationClientMaxSize(vServer::validateClientMaxSizeDirective(pair.second));
		break;

		case RETURN_DIR:
			loc.setLocationReturnPages(Location::setLocationReturnPages(pair.second));
		break;

		case ALLOWED_METHODS:
			loc.validateAllowedMethodsDirective(pair.second);
		break;

		case ALLOWED_CGI:
			loc.validateAllowedCgiDirective(pair.second);
		break;

		case ERROR_PAGE_DIR:
			loc.setLocationErrorPages(vServer::validateErrorPagesDirective(pair.second));
		break;

		default:
			throw ConfException("Invalid directive name: " + pair.first.lexem + " not found!");
		}
	}

	if (vServer.getServerLocations().find(locationPath) == vServer.getServerLocations().end())
		vServer.getServerLocations().emplace(locationPath, loc);
	else
		throw ConfException("Double location block detected.");
}


void	ParseConfig::validateServerBlockDirectives(vServer& serv) {

	std::pair< Token, std::vector<std::string>> pair = makeKeyValuePair();
	isSeenDirective(pair.first);
	switch (pair.first.type) {

		case LISTEN_DIR:
			serv.validateServerListen(pair.second);
		break;

		case SERVER_NAME_DIR:
			serv.validateServerNames(pair.second);
		break;

		case ROOT_DIR:
			serv.setServerRoot(vServer::onlyOneArgumentCheck(pair.second, "root"));
		break;

		case INDEX_DIR:
			serv.setServerIndex(pair.second);
		break;

		case AUTO_INDEX_DIR:
			serv.setServerAutoIndex(vServer::validateAutoIndexDirective(pair.second));
		break;

		case BODY_MAX_SIZE:
			serv.setServerClientMaxSize(vServer::validateClientMaxSizeDirective(pair.second));
		break;

		case ERROR_PAGE_DIR:
			serv.setServerErrorPages(vServer::validateErrorPagesDirective(pair.second));
		break;

		default:
			throw ConfException("Invalid directive name: " + pair.first.lexem + " not found!");
	}
}


std::pair< Token, std::vector<std::string>>	ParseConfig::makeKeyValuePair() {

	Token										key = _tokens[currToken];
	ssize_t										tokenLevel = _tokens[currToken].line_number;
	std::vector<std::string>					values;
	std::pair< Token, std::vector<std::string>>	pair;

	if (key.type == SEMICOLON) {
		throw ConfException(std::to_string(_tokens[currToken].line_number) + " :: This filed has no directive name.");
	}
	currToken ++; //sitch to the next token, potential value
	if (_tokens[currToken].type == SEMICOLON) {
		throw ConfException(std::to_string(_tokens[currToken].line_number) + " :: This filed has no arguments.");
	}
	while (_tokens[currToken].type != SEMICOLON) {
		std::cout << key.lexem << " " <<_tokens[currToken].lexem  << " " << _tokens[currToken].line_number << "\n";
		if (_tokens[currToken].line_number == tokenLevel)
			values.push_back(_tokens[currToken].lexem);
		else
			throw ConfException(std::to_string(_tokens[currToken].line_number - 1) + " :: No semicolon at the end of the line.");
			
		currToken++;
	}
	if (_tokens[currToken].type == SEMICOLON && _tokens[currToken].line_number != tokenLevel) {
			throw ConfException(std::to_string(_tokens[currToken].line_number - 1 ) + " :: No semicolon at the end of the line.");
	}
	pair = {key, values};
	return (pair);
}

