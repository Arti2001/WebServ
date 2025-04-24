#include "ParseConfig.hpp"


ParseConfig::ParseConfig(const char* file) : _file(file), depth(0), currToken(0) {

	_keywords[";"] = SEMICOLON;
	_keywords["{"] = OPENED_BRACE;
	_keywords["}"] = CLOSED_BRACE;


	_keywords["listen"] = LISTEN_DIR;
	_keywords["root"] = ROOT_DIR;
	_keywords["index"] = INDEX_DIR;
	_keywords["server-name"] = SERVER_NAME_DIR;
	_keywords["error-page"] = ERROR_PAGE_DIR;
	_keywords["server"] = SERVER_BLOCK;
	_keywords["location"] = LOCATION_BLOCK;
	_keywords["value"] = VALUE;
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

bool ParseConfig::isDirective(TokenType type) const {
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
	else
	std::cout << "file: '" << _file << "' is opend." << "\n";
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
		else
		new_string.push_back(c);
	}
	return (new_string);
}

std::vector<std::string>	ParseConfig::prepToToken() {

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



std::vector<std::string> split(const std::string& str) {

	std::vector<std::string>	lexemes;
	std::stringstream			iss(str);
	std::string					lexeme;

	while(iss >> lexeme)
	{
		lexemes.push_back(lexeme);
	}
	return (lexemes);
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






void	ParseConfig::tokenize(std::vector<std::string> roughData) {

	
	
	Token	token;
	std::vector<std::string>::iterator	it = roughData.begin();
	
	while (it != roughData.end()) {
		
		if ((*it)[0] == '#')
			it++;

		token.lexem = *it;
		if (_keywords.find(*it) != _keywords.end()) {
			token.type = _keywords[*it];
		}
		else
			token.type	= UNKNOWN;
		
		_tokens.push_back(token);
		it++;
	}
	//for (size_t i = 0; i < _tokens.size(); i++)
	//{
	//	std::cout << "lexeme: " + _tokens[i].lexem << "---> " << _tokens[i].type << "\n";
	//}
}






void	ParseConfig::TakeToken() {

	
	for (size_t i = 0; i < _tokens.size(); i++) {
		
		
		if (_tokens[i].type == SERVER_BLOCK) {
			if (_tokens[i + 1].type != OPENED_BRACE) {
				throw ConfException("Expected ' { ' after ' server '");
			}
			
			vServer	vserv;
			std::cout << "New vServer" << "\n";

			depth = 1;
			currToken += 2;
			parsVServer(vserv);
			_vServers.push_back(vserv);

		} else {
			throw ConfException("Unexpected token: '" + _tokens[i].lexem + "'");
		}

	}
	
}


void	ParseConfig::parsVServer( vServer& serv) {

	for (; depth > 0 && currToken < _tokens.size(); currToken++) {

		if (_tokens[currToken].type == CLOSED_BRACE) {
			depth--;
			continue;
		}
		if (isDirective(_tokens[currToken].type)) {
			parsServerBlock(serv);
		}
		else if (_tokens[currToken].type == LOCATION_BLOCK) {
			parsLocationBlock(serv);
		}

	}
	if (depth != 0)
		throw std::runtime_error("Error: unclosed server block");

}


std::pair< Token, std::vector<std::string>>	ParseConfig::makePair() {

	Token										key = _tokens[currToken];
	std::vector<std::string>					values;
	std::pair< Token, std::vector<std::string>>	pair;

	currToken +=1; //sitch to the next token, potential value

	if ( _tokens[currToken].type == SEMICOLON) {
		values.push_back("");// If token is ; that means key's value is an empty string
	}
	while (_tokens[currToken].type != SEMICOLON) {
		
		if (isDirective(_tokens[currToken].type)) {
			throw ConfException("Enclosed line: No semicolon at the end of a line!");
		}
		else if (_tokens[currToken].type == UNKNOWN) {
			values.push_back(_tokens[currToken].lexem);
		}
		currToken++;
	}
	pair = {key, values};
	return (pair);
}

void	ParseConfig::parsLocationBlock(vServer& serv) {

	Location	loc(serv);
	std::pair< Token, std::vector<std::string>> pair = makePair();;


	currToken++;
	loc.getPath(_tokens[currToken].lexem);
	
	switch (pair.first.type) {
		
		
		case ROOT_DIR:
			loc._root = onlyOneCheck(pair.second, "root");
		case INDEX_DIR:
			loc._index  = onlyOneCheck(pair.second, "index");
		case AUTO_INDEX_DIR:
			loc._auto_index = validAutoIndex(pair.second);
		case BODY_MAX_SIZE:
			loc._clientMaxSize = validClientMaxSize(pair.second);
		case ALLOWED_METHODS:
			loc._allowedMethods = validAllowedMethods(pair.second);
		case ERROR_PAGE_DIR:
			loc._errorPages = validErrorPages(pair.second);
		default:
			std::cout << "Not found for  location" << "\n";
	}
	serv.getLocations().push_back(loc);
}

void	ParseConfig::parsServerBlock(vServer& serv) {

	std::pair< Token, std::vector<std::string>> pair = makePair();;
	
	switch (pair.first.type) {

		case LISTEN_DIR:
			serv.setListen(pair.second);
		break;

		case SERVER_NAME_DIR:
			serv.setServerName(pair.second);
		break;

		case ROOT_DIR:
			serv.setRoot(pair.second);
		break;

		case INDEX_DIR:
			serv.setIndex(pair.second);
		break;

		case AUTO_INDEX_DIR:
			serv.setAutoIndex(pair.second);
		break;

		case BODY_MAX_SIZE:
			serv.setClientMaxSize(pair.second);
		break;

		case ALLOWED_METHODS:
			serv.setAllowedMethods(pair.second);
		break;

		case ERROR_PAGE_DIR:
			serv.setErrorPages(pair.second);
		break;

		default:
			std::cout << "Not found" << "\n";
	}
}

