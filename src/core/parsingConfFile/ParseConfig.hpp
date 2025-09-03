/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 19:17:08 by amysiv            #+#    #+#             */
/*   Updated: 2025/09/01 21:49:02 by amysiv           ###   ########.fr       */
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
	HASH,
	COMMENT,
	UNKNOWN
};




/**
* @brief A token constructor
* @param lineNumber  current line number
* @param word actual word(lexem)
* @param tokenType type of the token
*/
struct Token {
	Token() = default;
	Token(size_t	lineNumber, std::string word, TokenType tokenType);
	std::string	lexem;
	TokenType	type;
	ssize_t		line_number;

};


class ParseConfig {
	private:
		std::vector<Token>							_tokens; //vector of tokens
		std::unordered_map<std::string, TokenType>	_keywords; // map  ; = SEMICOLON;



		/**
		 * @brief Parses all directives and location blocks within a server block.
		 *
		 * Iterates over tokens in the server block, validating each server directive
		 * or location block, and updates the given vServer object.
		 *
		 * @param serv The server object to populate with configuration.
		 * @return void
		 *
		 * @throw ConfException If an unrecognized token or invalid configuration is found.
		 */
		void											parsevServerBlock(vServer& serv);




		/**
		 * @brief Parses and validates a directive inside a server block.
		 *
		 * Reads the next key–value pair, checks for duplicate non-repeatable
		 * directives, and applies the configuration to the given vServer.
		 *
		 * @param serv The server object to update with the directive.
		 * @return void
		 *
		 * @throw ConfException If the directive is invalid, duplicated,
		 *        or not recognized.
		 */
		void											validateServerBlockDirectives(vServer& serv);




		/**
		 * @brief Parses and validates directives inside a location block.
		 *
		 * Extracts the location path, processes each directive within the block,
		 * and updates the given vServer with the parsed Location configuration.
		 *
		 * @param vServer The virtual server object to which the location belongs.
		 * @return void
		 *
		 * @throw ConfException If a directive is invalid, duplicated, or the
		 *        location block is malformed (e.g., missing brace or duplicate path).
		 */
		void											validateLocationBlockDirectives(vServer& serv);




		/**
		 * @brief Extracts the path following a location directive.
		 *
		 * @return The extracted location path as a string.
		 *
		 * @throw ConfException If the path does not start with '/' or
		 *        if the opening brace is missing.
		 */
		std::string										findLocationPath(void);




		/**
		 * @brief Builds a key–value pair from the current tokens.
		 *
		 * Extracts a directive token as the key and its arguments as values,
		 * ensuring correct syntax (semicolon termination, same line, etc.).
		 *
		 * @return A pair consisting of the directive token and its arguments.
		 *
		 * @throw ConfException If the directive has no name, no arguments,
		 *        or is missing a terminating semicolon.
		 */
		std::pair<Token, std::vector<std::string>>		makeKeyValuePair(void);




		/**
		 * @brief Creates a default virtual server. Uses default values from the vServer constructor
		 * @param void
		 * @return virtual server object
		 */
		const vServer									createDefaultConfig(void);




		/**
		 * @brief Registers a directive and ensures it is not duplicated.
		 *
		 * @param directive The directive token to check.
		 * @return void
		 *
		 * @throw ConfException If the directive was already seen and must be unique.
		 */
		void											isSeenDirective(Token directive);




		/**
		 * @brief Checks whether a configuration directive must not be repeated.
		 * Determines if the given token type corresponds to a directive
		 * that should only appear once in the configuration file.
		 * @param type The token type to check.
		 * @return true if the directive must not be repeated, false otherwise.
		 */
		bool											noRepeatDirective(TokenType type) const;




		/**
		* @brief	Checks if all braces are closed.
		*/
		bool											validBrace(void);




		/**
		* @brief This method  checks if token is a directive
		* @param  Token's type
		* @return true if token is a directive, if no false is returned.
		*/
		bool											isTokenDirective(TokenType type) const;




	public:
		int								depth; // depth counter
		size_t							currToken; // current token countre to iterate on _tokens vector
		std::unordered_set<TokenType>	seenDirectives; // set register of seen directives

		ParseConfig();
		~ParseConfig();



		/**
		 * @brief Prepares raw configuration file data for tokenization.
		 *
		 * Reads a configuration file line by line, ignores empty lines and # type comments,
		 * Stores the lexemes along with their line numbers in a map for further processing.
		 *
		 * @param configFile Input file stream of the configuration file.
		 * @return std::map<size_t, std::vector<std::string>>
		 *         Map where the key is the line number and the value is a vector of lexemes.
		 */
		std::map<size_t, std::vector<std::string>>		prepToTokenizeConfigData(std::ifstream& configFile);




		/**
		 * @brief Converts raw configuration lines into a sequence of tokens.
		 *
		 * Iterates through the given map of line numbers to lexemes and determines the type
		 * of each lexeme based on predefined keywords. Unknown lexemes are marked as UNKNOWN,
		 * and lexemes following a semicolon or comment are treated as comments. Each lexeme
		 * is stored as a Token in the internal _tokens vector.
		 *
		 * @param lineNumbLexemes Map of line numbers to vectors of lexemes for that line.
		 * @return void
		 */
		void											tokenizeConfigData(std::map<size_t, std::vector<std::string>> lineNumbLexemes);




		/**
		 * @brief Parses all tokens from the configuration file into virtual server objects.
		 *
		 * Iterates through the token stream, validating braces and parsing each server block.
		 * For each server block, parses its directives and location blocks, populating the
		 * given vector of vServer objects. If a default location ("/") is missing, it is added.
		 * Ensures no unexpected tokens are present outside server blocks.
		 *
		 * @param _vServers Vector to populate with parsed virtual server configurations.
		 * @return void
		 *
		 * @throw ConfException If braces are unbalanced, a server block is malformed,
		 *        or unexpected tokens are found.
		 */
		void											parseConfigFileTokens(std::vector<vServer>& _vServers);





		/**
		* @brief exception class
		* @param message to display.
		*/
		class ConfException : public std::exception {
			private:
				std::string	_message;
			public:
				ConfException(const std::string& message);
				const char*	what() const noexcept override;
		};
	};

	std::vector<std::string>							split(const std::string& str);




	std::ostream& 										operator<<(std::ostream& os, const vServer& server);




	std::ostream&										operator<<(std::ostream& os, const std::vector<vServer>& servers);

#endif