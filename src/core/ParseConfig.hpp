#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include "Server.hpp"
#include <unordered_map>
#include <fstream>
#include "vServer.hpp"


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

	
	SERVER_BLOCK,
	LOCATION_BLOCK,

	
	PATH,
	VALUE,
	UNKNOWN
};



struct Token {

	std::string	lexem;
	TokenType	type;
	ssize_t		line_number;

};


class ParseConfig {
	private:
	std::ifstream												_configfile;
	std::string													_file;
	std::vector<Token>											_tokens;
	std::vector<vServer>										_vServers;
	std::unordered_map<std::string, TokenType>					_keywords;

	public:
		int			depth;
		size_t		currToken;


		ParseConfig(const char* path);
		~ParseConfig();


		
		bool						openConfigFile();
		bool						isTokenDirective(TokenType type) const;
		std::vector<std::string>	prepToTokenizeConfigData();
		void						tokenizeConfigData(std::vector<std::string> roughData);
		bool						validBrace();
		void						validateConfigFileTokens();
		void						parseVirtualServerBlock(vServer& serv);
		void						validateServerBlockDirectives(vServer& serv);
		void						validateLocationBlockDirectives(vServer& serv);
		void						printServerMapH();
		void						addToMap();
		
		
		std::pair< Token, std::vector<std::string>>						makeKeyValuePair();
		
		
		
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

#endif