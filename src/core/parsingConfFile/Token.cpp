#include "ParseConfig.hpp"

bool	isComment(const std::string& line) {
	
	for (char c : line) {
		if (isspace(c)) {
			continue;
		}
		return(c == '#'); 
		
	}
	return (false);
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

std::map<size_t, std::vector<std::string>>	ParseConfig::prepToTokenizeConfigData(std::ifstream& configFile) {

	std::vector<std::string>					lexemes;
	std::map<size_t, std::vector<std::string>>	lineNumbLexemes;
	std::string									line, spacedLine;
	int											lineCounter = 1;

	while (std::getline(configFile, line)) {
		if (line.empty() || isComment(line)) {
			lineCounter++;
			continue;
		}
		spacedLine = addSpace(line);
		lineNumbLexemes[lineCounter] = split(spacedLine);
		lineCounter++;
	}
	configFile.close();
	return (lineNumbLexemes);
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


void	ParseConfig::tokenizeConfigData(std::map<size_t, std::vector<std::string>> lineNumbLexemes) {

	TokenType													tokenType;
	for(std::map<size_t, std::vector<std::string>>::iterator	itMap = lineNumbLexemes.begin(); itMap != lineNumbLexemes.end(); itMap++) {
		
		TokenType	prevTokenType = UNKNOWN;
		size_t		lineNumber = itMap->first;

		for (const std::string& word : itMap->second) {

			if (_keywords.find(word) != _keywords.end()) {
				tokenType = _keywords[word];
			}
			else {
				if (prevTokenType == SEMICOLON || prevTokenType == COMMENT) {
					tokenType = COMMENT;
				}
				else
					tokenType = UNKNOWN;
			}
			_tokens.push_back(Token(lineNumber, word, tokenType));
			prevTokenType = tokenType;
		}
	}
}
