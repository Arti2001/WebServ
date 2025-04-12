#include "ParseConfig.hpp"


ParseConfig::ParseConfig(const char* file) : _file(file) {

	_keywords ["="] = EQUAL;
	_keywords [";"] = SEMICOLON;
	_keywords ["{"] = BRACE_LEFT;
	_keywords ["/"] = BACK_SLASH;
	_keywords ["}"] = BRACE_RIGHT;
	
	_keywords["server"] = SERVER;
	_keywords["listen"] = LISTEN;
	_keywords["location"] = LOCATION;
	_keywords["error_page"] = ERROR_PAGE;
	_keywords["server_name"] = SERVER_NAME;
}

ParseConfig::ConfException::ConfException(const std::string& msg) : _message(msg) {
	
}


ParseConfig::~ParseConfig() {
	
	if (_configfile.is_open())
	_configfile.close();
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

Token ParseConfig::tokenize(std::string word) {
	
	Token	token;
	
	if (_keywords.find(word) != _keywords.end()) {
		token.type = _keywords[word];
		token.lexem = word;
	}
	else {
		token.type = IDENTIFIER;
		token.lexem = word;
	}
	return (token);
}


std::string	addSpace(const std::string& str) {

	std::string	new_string;
	
	for (char c : str) {
	
		if (c == '=' || c == ';' | c == '{' | c == '}' | c == '/') {
			new_string.push_back(' ');
			new_string.push_back(c);
			new_string.push_back(' ');
		}
		else
			new_string.push_back(c);
	}
	return (new_string);
}



std::string	prepToToken(const std::string& str) {

	std::string line;
	std::string	readyToSplit;

	std::getline(std::cin, line);
	readyToSplit = addSpace(line);

	return
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