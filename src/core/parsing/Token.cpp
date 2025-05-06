#include "ParseConfig.hpp"

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
		if (line.empty() || line.at(0) == '#') {
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


    std::map<size_t, std::vector<std::string>>::iterator	itMap = lineNumbLexemes.begin();
	size_t													lineNumber = 0;
	std::string												word;
	TokenType												tokenType;


	for (; itMap != lineNumbLexemes.end();itMap++)
	{
		lineNumber = itMap->first;
		for(std::vector<std::string>::iterator itVector = itMap->second.begin(); itVector !=  itMap->second.end(); itVector++) {
			word = *itVector;
			if (_keywords.find(*itVector) != _keywords.end())
				tokenType = _keywords[*itVector];
			else
				tokenType = UNKNOWN;
			_tokens.push_back(Token(lineNumber, word, tokenType));
		}
	}
}