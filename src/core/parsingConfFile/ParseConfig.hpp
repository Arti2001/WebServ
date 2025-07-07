/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 13:09:10 by vshkonda          #+#    #+#             */
/*   Updated: 2025/07/06 18:30:09 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	UPLOAD_PATH,
	ROOT_DIR,
	INDEX_DIR,
	SERVER_NAME_DIR,
	ERROR_PAGE_DIR,
	AUTO_INDEX_DIR,
	BODY_MAX_SIZE,
	ALLOWED_METHODS,
	ALLOWED_CGI,
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
		std::vector<Token>							_tokens;
		std::unordered_map<std::string, TokenType>	_keywords;
		
	public:
		int								depth;
		size_t							currToken;
		std::unordered_set<TokenType>	seenDirectives;

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
		const vServer							createDefaultConfig(void);
		void											isSeenDirective(Token directive);
		bool											noRepeatDirective(TokenType type) const;
		
	
		//helper
		bool											validBrace();
		bool											isTokenDirective(TokenType type) const;

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