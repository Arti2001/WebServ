#include "ParseConfig.hpp"


ParseConfig::ParseConfig(const char* file) : _file(file), depth(0), currToken(0) {

	_keywords[";"] = SEMICOLON;
	_keywords["{"] = OPENED_BRACE;
	_keywords["}"] = CLOSED_BRACE;


	_keywords["listen"] = LISTEN_DIR;
	_keywords["root"] = ROOT_DIR;
	_keywords["index"] = INDEX_DIR;
	_keywords["server-name"] = SERVER_NAME_DIR;
	_keywords["error-pages"] = ERROR_PAGE_DIR;
	_keywords["server"] = SERVER_BLOCK;
	_keywords["location"] = LOCATION_BLOCK;
	_keywords["auto-index"] = AUTO_INDEX_DIR;
	_keywords["client-max-size"] = BODY_MAX_SIZE;
	_keywords["allowed-methods"] = ALLOWED_METHODS;
	
}

ParseConfig::ConfException::ConfException(const std::string& msg) : _message(msg) {
	
}

ParseConfig::~ParseConfig() {
	
	if (_configfile.is_open())
	_configfile.close();
}

bool ParseConfig::isTokenDirective(TokenType type) const {
	return (type == LISTEN_DIR || type == ROOT_DIR || 
			type == INDEX_DIR || type == SERVER_NAME_DIR || 
			type == ERROR_PAGE_DIR || type == AUTO_INDEX_DIR ||
			type == BODY_MAX_SIZE || type == ALLOWED_METHODS);
}


const char*	ParseConfig::ConfException::what() const noexcept {
	
	return (_message.c_str());
}


bool	ParseConfig::openConfigFile() {
	
	_configfile.open(_file);
	if (!_configfile) {

		throw (ConfException("Failed to open " + _file));
	}
	else{

		std::cout << "file: '" << _file << "' is opend." << "\n";
	}
	return (true);
}




std::string	addSpace(const std::string& str) {

	std::string	new_string;
	
	for (char c : str) {
		
		if (c == '=' || c == ';' || c == '{' || c == '}') {
			new_string.push_back(' ');
			new_string.push_back(c);
			new_string.push_back(' ');
		}
		else {

			new_string.push_back(c);
		}
	}
	return (new_string);
}

std::vector<std::string>	ParseConfig::prepToTokenizeConfigData() {

	std::ostringstream			oss;
	std::vector<std::string>	lexemes;
	std::string					line, spaced;

	while (std::getline( _configfile, line)) {
		oss << line;
	}

	if (_configfile.eof()) {
		std::cout<< "End of the file!"<< "\n";
	}

	spaced = addSpace(oss.str());
	lexemes = split(spaced);
	_configfile.close();

	return (lexemes);
}



std::vector<std::string>	split(const std::string& str) {

	std::vector<std::string>	lexemes;
	std::stringstream			iss(str);
	std::string					lexeme;

	while(iss >> lexeme)
	{
		lexemes.push_back(lexeme);
	}
	return (lexemes);
}

//bool	ParseConfig::validBrace() {

//	int	lvl = 0;

//	for (size_t i = 0; i < _tokens.size(); i++) {

//		if (_tokens[i].lexem == "{") {
//			lvl++;
//		}
//		else if(_tokens[i].lexem == "}") {
//			lvl--;
//		}
//	}

//	if(lvl == 0) {
//		return (true);
//	}
//	return (false);
//}


void	ParseConfig::tokenizeConfigData(std::vector<std::string> roughData) {

	Token	token;
	std::vector<std::string>::iterator	it = roughData.begin();
	
	while (it != roughData.end()) {
		
		if ((*it)[0] == '#')
			it++;

		token.lexem = *it;
		if (_keywords.find(*it) != _keywords.end()) {

			token.type = _keywords[*it];
		}
		else {

			token.type	= UNKNOWN;
		}
		
		_tokens.push_back(token);
		it++;
	}
}


void	ParseConfig::parsConfigFileTokens() {

	
	for (; currToken < _tokens.size(); currToken++) {
		
		
		if (_tokens[currToken].type == SERVER_BLOCK) {
			if (_tokens[currToken + 1].type != OPENED_BRACE) {
				throw ConfException("Expected ' { ' after ' server '");
			}
			
			vServer	vserv;
			depth = LEVEL;
			currToken += 2;
			parsVirtualServerBlock(vserv);
			_vServers.push_back(vserv);
		}
		else {
			throw ConfException("Unexpected token: '" + _tokens[currToken].lexem + "'");
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

	os << "Allowed Methods:              ";
	for (size_t i = 0; i < server.getServerAllowedMethods().size(); ++i)
	os << server.getServerAllowedMethods()[i] << (i + 1 < server.getServerAllowedMethods().size() ? ", " : "\n");
	os << "  Error Pages:\n";
	const std::unordered_map<int, std::string>& errorPages = server.getServerErrorPages();
	for (std::unordered_map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		os << "    " << it->first << ": " << it->second << "\n";
	
	os << "------------------ Locations ------------------\n";
	const std::vector<Location>& locs = server.getServerLocations();
	for (size_t i = 0; i < locs.size(); ++i) {
		const Location& loc = locs[i];
		os << "Location [" << i << "]\n";
		os << "  Path:           " << loc._locationPath << "\n";
		os << "  Root:           " << loc._locationRoot << "\n";
		os << "  Index:          " << loc._locationIndex << "\n";
		os << "  AutoIndex:      " << loc._locationAutoIndex << "\n";
		os << "  Max Body Size:  " << loc._locationClientMaxSize << "\n";
		os << "  Allowed Methods:";
		for (size_t j = 0; j < loc._locationAllowedMethods.size(); ++j)
			os << " " << loc._locationAllowedMethods[j];
		os << "\n";

	}

	os << "=======================================================\n";
	return os;
}


void	ParseConfig::parsVirtualServerBlock( vServer& serv) {
	
	for (;depth > 0; currToken++) {

		if (_tokens[currToken].type == CLOSED_BRACE) {
			
			std::cout << "Clousing brace is encauntered"<< "\n";
			depth--;
		}
		else if (isTokenDirective(_tokens[currToken].type)) {
			validateServerBlockDirectives(serv);
		}
		else if (_tokens[currToken].type == LOCATION_BLOCK) {
			
			depth += LEVEL;
			validateLocationBlockDirectives(serv);
		}
	}
	if (depth != 0)
		throw std::runtime_error("Error: unclosed server block");
	else if (_tokens[currToken].type == SERVER_BLOCK)
		currToken--;
	
}

void ParseConfig::validateLocationBlockStructure() {

	int directiveIndex = currToken;//we need to loop and check if the location block structure is correct
	for (;_tokens[directiveIndex].type != CLOSED_BRACE; directiveIndex++) {
		if (isTokenDirective(_tokens[directiveIndex].type)) {
			
			return;
		}
		else
		throw ConfException("Wrong syntax: Unexpected token:" + _tokens[directiveIndex].lexem);
	}
	if (_tokens[directiveIndex].type == CLOSED_BRACE)
		throw ConfException("Empty location block");
}


Location	ParseConfig::createLocationInstance(const vServer& server) {
	
	currToken++;//move to the path
	
	std::string	locationPath = Location::setLocationPath(_tokens[currToken].lexem);
	
	currToken += 2;//move to a directive
	
	validateLocationBlockStructure();
	Location	loc(server);
	loc._locationPath = locationPath;
	return (loc);
}


void	ParseConfig::validateLocationBlockDirectives(vServer& server) {
	
	
	Location	loc = createLocationInstance(server);
	
	
	for (; _tokens[currToken].type != CLOSED_BRACE; currToken++) {
		
		std::pair<Token, std::vector<std::string>> pair = makeKeyValuePair();

		switch (pair.first.type) {
			
			case ROOT_DIR:
				loc._locationRoot = vServer::onlyOneArgumentCheck(pair.second, "root");
			break;
			
			case INDEX_DIR:
				loc._locationIndex = vServer::onlyOneArgumentCheck(pair.second, "index");
			break;
				
			case AUTO_INDEX_DIR:
				loc._locationAutoIndex = vServer::validateAutoIndexDirective(pair.second);
			break;
				
			case BODY_MAX_SIZE:
				loc._locationClientMaxSize = vServer::validateClientMaxSizeDirective(pair.second);
			break;
				
			case ALLOWED_METHODS:
				loc._locationAllowedMethods = vServer::validateAllowedMethodsDirective(pair.second);
			break;
				
			case ERROR_PAGE_DIR:
				loc._locationErrorPages = vServer::validateErrorPagesDirective(pair.second);
			break;
				
				default:
					std::cout << "Not found for  location" << "\n";
			}
		}
		
		server.getServerLocations().push_back(loc);

	}
	
	void	ParseConfig::validateServerBlockDirectives(vServer& serv) {
		
		std::pair< Token, std::vector<std::string>> pair = makeKeyValuePair();
		switch (pair.first.type) {
		
		case LISTEN_DIR:
			serv.setServerListen(pair.second);
		break;
		
		case SERVER_NAME_DIR:
			serv.setServerNames(pair.second);
		break;
		
		case ROOT_DIR:
			serv.setServerRoot(pair.second);
		break;
		
		case INDEX_DIR:
			serv.setServerIndex(pair.second);
		break;
		
		case AUTO_INDEX_DIR:
			serv.setServerAutoIndex(pair.second);
		break;
		
		case BODY_MAX_SIZE:
			serv.setServerClientMaxSize(pair.second);
		break;
		
		case ALLOWED_METHODS:
			serv.setServerAllowedMethods(pair.second);
		break;
		
		case ERROR_PAGE_DIR:
			serv.setServerErrorPages(pair.second);
		break;
		
		default:
			std::cout << "Not found" << "\n";
	}
}


std::pair< Token, std::vector<std::string>>	ParseConfig::makeKeyValuePair() {

	Token										key = _tokens[currToken];
	std::vector<std::string>					values;
	std::pair< Token, std::vector<std::string>>	pair;

	currToken ++; //sitch to the next token, potential value

	if ( _tokens[currToken].type == SEMICOLON) {
		throw ConfException("This field '" + key.lexem + "' can NOT remain empty.");
	}
	while (_tokens[currToken].type != SEMICOLON) {


		if (isTokenDirective(_tokens[currToken].type)) {
			throw ConfException("Enclosed line: No semicolon at the end of a line!");
		}
		else if (_tokens[currToken].type == UNKNOWN) {
			values.push_back(_tokens[currToken].lexem);
		}
		else {
			throw ConfException("Unexpected token type inside key-value pairing.");
		}
		currToken++;
	}
	pair = {key, values};

	return (pair);
	//curr toke is a secolon
}