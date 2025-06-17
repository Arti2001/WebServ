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
	_keywords["autoindex"] = AUTO_INDEX_DIR; // by convention, autoindex is a directive
	_keywords["upload_path"] = UPLOAD_PATH;
	_keywords["server_name"] = SERVER_NAME_DIR; // by convention, server_name is a directive
	_keywords["error_pages"] = ERROR_PAGE_DIR; // by convention, error_pages is a directive
	_keywords["allowed_methods"] = ALLOWED_METHODS;
	_keywords["client_max_body_size"] = BODY_MAX_SIZE; // by convention, client_max_body_size is a directive
	// need to add keywords for upload path, cgi in the location block
}

ParseConfig::ConfException::ConfException(const std::string& msg) : _message(msg) {

}


const char*	ParseConfig::ConfException::what() const noexcept {
	
	return (_message.c_str());
}

//std::vector<vServer>	ParseConfig::getVSevers() const {

//	return (_vServers);
//}


Token::Token() {}

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
			type == RETURN_DIR || type == UPLOAD_PATH);
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

void	ParseConfig::parseConfigFileTokens(std::vector<vServer>& _vServers) {
	
	if (!validBrace())
		throw ConfException("Floating closing brace in the configuration file");
	for (; currToken < _tokens.size();currToken++) {
		
		
		if (_tokens[currToken].type == SERVER_BLOCK) {
			if (_tokens[currToken + 1].type != OPENED_BRACE) {
				throw ConfException("Expected ' { ' after ' server '");
			}
			
			vServer	vserv;
			depth = LEVEL;
			currToken += 2;
			parsevServerBlock(vserv);
			_vServers.push_back(vserv);
		}
		else {
			throw ConfException("Alien object is detected at the line: " + std::to_string(_tokens[currToken].line_number));
		}
	}
	std::cout << _vServers;
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
	for (size_t i = 0; i < server.getServerNames().size(); i++)
		os << server.getServerNames()[i] << (i + 1 < server.getServerNames().size() ? ", " : "\n");

	os << "Client Max Body Size:         " << server.getServerClientMaxSize() << "\n";
	os << "Root:                         " << server.getServerRoot() << "\n";
	os << "Index:                        " << server.getServerIndex() << "\n";
	os << "AutoIndex:                    " << server.getServerAutoIndex() << "\n";
	os << "  Error Pages:\n";
	const std::unordered_map<int, std::string>& errorPages = server.getServerErrorPages();
	for (std::unordered_map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		os << "    " << it->first << ": " << it->second << "\n";
	
	os << "------------------ Locations ------------------\n";
	const std::vector<Location>& locs = server.getServerLocations();
	for (size_t i = 0; i < locs.size(); ++i) {
		const Location& loc = locs[i];
		os << "Location [" << i << "]\n";
		os << "  Path:           " << loc.getLocationPath() << "\n";
		os << "  Root:           " << loc.getLocationRoot() << "\n";
		os << "  Index:          " << loc.getLocationIndex() << "\n";
		os << "  AutoIndex:      " << loc.getLocationAutoIndex() << "\n";
		os << "  UploadPath:     " << loc.getLocationUploadPath() << "\n";
		os << "  Max Body Size:  " << loc.getLocationClientMaxSize() << "\n";
		os << "  Allowed Methods:";
		for (size_t j = 0; j < loc.getLocationAllowedMethods().size(); ++j)
			os << " " << loc.getLocationAllowedMethods()[j];
		os << "\n";

	}

	os << "=======================================================\n";
	return os;
}


void	ParseConfig::	parsevServerBlock( vServer& serv) {
	
	while (depth > 0) {

		if (_tokens[currToken].type == CLOSED_BRACE) {
			std::cout << "Closing brace is encountered"<< "\n";
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


void	ParseConfig::validateLocationBlockDirectives(vServer& server) {
	
Location	loc(server);

std::string locationPath = findLocationPath();
loc.setLocationPath(locationPath);
	
for (; _tokens[currToken].type != CLOSED_BRACE; currToken++) {
	
	std::pair<Token, std::vector<std::string>> pair = makeKeyValuePair();

	switch (pair.first.type) {
		case ROOT_DIR:
			loc.setLocationRoot(vServer::onlyOneArgumentCheck(pair.second, "root"));
		break;
		
		case INDEX_DIR:
			loc.setLocationIndex(vServer::onlyOneArgumentCheck(pair.second, "index"));
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
			
		case ERROR_PAGE_DIR:
			loc.setLocationErrorPages(vServer::validateErrorPagesDirective(pair.second));
		break;
			
		default:
			throw ConfException("Invalid directive name: " + pair.first.lexem + " not found!");
		}
	}
	server.getServerLocations().push_back(loc);
}

// it is better to have validation inside of the vServer class
void	ParseConfig::validateServerBlockDirectives(vServer& serv) {
		
std::pair< Token, std::vector<std::string>> pair = makeKeyValuePair();

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
		serv.setServerIndex(vServer::onlyOneArgumentCheck(pair.second, "index"));
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