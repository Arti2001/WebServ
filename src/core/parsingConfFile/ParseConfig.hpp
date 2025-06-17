#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

class vServer;
class Location;


#include <unordered_map>
#include "vServer.hpp"
#include <fstream>
#include "../ServerManager.hpp"

#define LEVEL 1


enum TokenType {
	
	SEMICOLON,
	OPENED_BRACE,
	CLOSED_BRACE,
	LISTEN_DIR,
	ROOT_DIR,
	INDEX_DIR,
	SERVER_NAME_DIR,
	ERROR_PAGE_DIR,
	AUTO_INDEX_DIR,
	BODY_MAX_SIZE,
	ALLOWED_METHODS,
	RETURN_DIR,
	SERVER_BLOCK,
	LOCATION_BLOCK,
	COMMENT,
	UNKNOWN
};

struct Token {

	Token(size_t	lineNumber, std::string word, TokenType tokenType);
	Token();
	std::string	lexem;
	TokenType	type;
	ssize_t		line_number;

}; 


class ParseConfig {
	private:
		std::vector<Token>											_tokens;
		std::unordered_map<std::string, TokenType>					_keywords;
		//std::vector<vServer>										_vServers;

	public:
		int			depth;
		size_t		currToken;

		ParseConfig();
		~ParseConfig();
		
		bool											openConfigFile();
		std::map<size_t, std::vector<std::string>>		prepToTokenizeConfigData(std::ifstream& configFile);
		void											tokenizeConfigData(std::map<size_t, std::vector<std::string>> lineNumbLexemes);
		void											parseConfigFileTokens(std::vector<vServer>& _vServers);
		void											parsevServerBlock(vServer& serv);
		void											validateServerBlockDirectives(vServer& serv);
		void											validateLocationBlockDirectives(vServer& serv);
		void											validateLocationBlockStructure();
		std::string										findLocationPath();
		std::pair< Token, std::vector<std::string>>		makeKeyValuePair();
		
		
		//helper
		bool											validBrace();
		bool											isTokenDirective(TokenType type) const;


		////getters
		//std::vector<vServer>		getVSevers( void ) const;
		
		
		class ConfException : public std::exception {
			private:
				std::string	_message;
			public:
				ConfException(const std::string& message);
				const char*	what() const noexcept override;
		};
		
	};
	std::vector<std::string>	split(const std::string& str);
	std::ostream& operator<<(std::ostream& os, const vServer& server);
	std::ostream& operator<<(std::ostream& os, const std::vector<vServer>& servers);

#endif