#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include "Server.hpp"
#include <unordered_map>
#include <fstream>

enum	tokenType {

	//Single-character tokens
	SEMICOLON,
	BRACE_LEFT,
	BRACE_RIGHT,
	BACK_SLASH,
	EQUAL,

	//Literals
	IDENTIFIER,
	STRING,
	NUMBER,

	//Kay-Words
	SERVER, LISTEN,
	LOCATION,
	ERROR_PAGE, SERVER_NAME
};

struct Token {

	std::string	lexem;
	tokenType	type;
};




class ParseConfig {
	private:
		std::unordered_map<std::string, tokenType>	_keywords;
		std::ifstream								_configfile;
		std::string									_file;


	public:
		ParseConfig(const char* path);
		~ParseConfig();


		class ConfException : public std::exception {
			private:
				std::string	_message;
			public:
				ConfException(const std::string& message);
				const char*	what() const noexcept override;
			};


		bool						openConfigFile();
		Token						tokenize(std::string word);
		std::string					prepToToken(const std::string& str);
	};
	
	
	std::vector<std::string>	split(const std::string& str);


#endif