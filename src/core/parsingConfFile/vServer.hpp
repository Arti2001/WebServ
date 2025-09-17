/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 13:09:20 by vshkonda          #+#    #+#             */
/*   Updated: 2025/09/17 10:50:33 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef vServer_HPP
#define vServer_HPP




#define MAX_PORT_NUMB 65535
#define MIN_PORT_NUMB 1
#define MAX_SERVER_NAME_NUMB 2
#define MAX_ARG	1
#define MAX_CLIENT_BODY_SIZE 10
#define MIN_CLIENT_BODY_SIZE 1
#define MAX_ARG_ERROR_PAGE	2




#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <regex>
#include <set>
#include <exception>
#include "ParseConfig.hpp"
#include "LocationConfig.hpp"




class vServer {
	private:
		std::string								_vServerIp;
		std::string								_vServerPort;
		std::string								_vServerIpPort;
		std::unordered_set<std::string>			_vServerNames;
		std::map<std::string, Location>			_vServerLocations;
		std::string								_vServerRoot;
		std::vector<std::string>				_vServerIndex;
		bool									_vServerAutoIndex;
		uint64_t								_vServerClientMaxSize;
		std::unordered_map<int, std::string>	_vServerErrorPages;

	public:
		vServer();
		~vServer();
		/**
		 * @name Server Configuration Getters
		 * @brief Accessors for server configuration settings.
		 *
		 * These methods provide read-only access to the server’s runtime configuration.
		 * @{
		 */

		/// @brief Returns whether autoindexing is enabled for the server.
		bool getServerAutoIndex(void) const;

		/// @brief Returns the server’s IP address as a string.
		std::string getServerIp(void) const;

		/// @brief Returns the server’s listening port as a string.
		std::string getServerPort(void) const;

		/// @brief Returns the combined "IP:Port" string of the server.
		std::string getServerIpPort(void) const;

		/// @brief Returns the maximum allowed client request body size (in bytes).
		uint64_t getServerClientMaxSize(void) const;

		/// @brief Returns the server’s root directory path.
		std::string getServerRoot(void) const;

		/// @brief Returns the set of server names (hostnames) defined.
		std::unordered_set<std::string> getServerNames(void) const;

		/// @brief Returns the list of index files to look for in directories.
		std::vector<std::string> getServerIndex(void) const;

		/// @brief Returns a modifiable reference to the server’s location blocks.
		std::map<std::string, Location>& getServerLocations();

		/// @brief Returns a read-only reference to the server’s location blocks.
		const std::map<std::string, Location>& getServerLocations() const;

		/// @brief Returns the mapping of error codes to error page paths.
		std::unordered_map<int, std::string> getServerErrorPages(void) const;

		/** @} */



	/**
	 * @name Server Configuration Setters
	 * @brief Mutators for server configuration settings.
	 *
	 * These methods allow modifying the server’s runtime configuration.
	 * @{
	 */

	/// @brief Sets the server’s location blocks.
	void setServerLocations(const std::map<std::string, Location>& loc);

	/// @brief Sets the server’s root directory path.
	void setServerRoot(const std::string& path);

	/// @brief Sets the list of index files to look for in directories.
	void setServerIndex(const std::vector<std::string>& index);

	/// @brief Enables or disables directory autoindexing.
	void setServerAutoIndex(const int mode);

	/// @brief Sets the maximum allowed client request body size (in bytes).
	void setServerClientMaxSize(const uint64_t size);

	/// @brief Sets the mapping of error codes to error page paths.
	void setServerErrorPages(const std::unordered_map<int, std::string>& pages);

	/** @} */





	/**
	 * @name Server Configuration Validators
	 * @brief Validation utilities for server and location directives.
	 *
	 * These methods check and enforce the correctness of configuration
	 * directives for the server and location blocks.
	 * @{
	 */

	/// @brief Validates the `listen` directive (IP/port format).
	/// @param addressVector Vector containing address and port tokens.
	void validateServerListen(const std::vector<std::string>& addressVector);

	/// @brief Validates the list of server names.
	/// @param names Vector of server names (may be modified if invalid).
	void validateServerNames(std::vector<std::string>& names);

	/** @} */




	/**
	 * @name Common Validators
	 * @brief Shared validation functions for both server and location classes.
	 *
	 * These are static helpers used across configuration parsing.
	 * @{
	 */

	/// @brief Validates the `autoindex` directive.
	/// @param flagVector Vector containing the directive arguments.
	/// @return `true` if autoindex is enabled, `false` otherwise.
	static bool validateAutoIndexDirective(const std::vector<std::string>& flagVector);

	/// @brief Validates and converts the `client_max_body_size` directive.
	/// @param sizeVector Vector containing the size argument (supports M/G suffix).
	/// @return Parsed size in bytes.
	static uint64_t validateClientMaxSizeDirective(const std::vector<std::string>& sizeVector);

	/// @brief Validates the `error_page` directive.
	/// @param errorPagesVector Vector containing error codes and paths.
	/// @return Mapping of error codes to error page paths.
	static std::unordered_map<int, std::string> validateErrorPagesDirective(const std::vector<std::string>& errorPagesVector);

	/// @brief Ensures that only one argument is provided for a directive.
	/// @param pathVector Vector of directive arguments.
	/// @param directiveName Name of the directive (for error reporting).
	/// @return Reference to the single valid argument.
	/// @throws std::runtime_error if more than one argument is provided.
	static const std::string& onlyOneArgumentCheck(const std::vector<std::string>& pathVector, std::string directiveName);

	/** @} */

};

bool	isNumber(std::string number);

#endif