/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 13:09:00 by vshkonda          #+#    #+#             */
/*   Updated: 2025/09/01 21:12:34 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */




#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP




#include "vServer.hpp"




/**
 * @class Location
 * @brief Represents configuration for a specific location block inside a server configuration.
 *
 * A Location object stores path-specific configuration directives such as
 * root directory, index files, allowed methods, upload path, and error pages.
 * Each location can override server-level settings.
 */
class Location {
	private:
		std::string _locationPath;
		std::string _locationUploadPath;
		std::string _locationRoot;
		std::vector<std::string> _locationIndex;
		int _locationAutoIndex;
		unsigned _locationClientMaxSize;
		std::unordered_set<std::string> _locationAllowedMethods;
		std::map<std::string, std::string> _locationAllowedCgi;
		std::pair<int, std::string> _locationReturnPages;
		std::unordered_map<int, std::string> _locationErrorPages;

	public:
		/**
		 * @brief Default constructor for Location.
		 */
		Location();

		/**
		 * @brief Destructor for Location.
		 */
		~Location();

		/**
		 * @brief Construct Location from a vServer object.
		 * @param serv Reference to vServer configuration.
		 */
		Location(const vServer& serv);

		/**
		 * @brief Copy constructor.
		 * @param other Location object to copy from.
		 */
		Location(const Location& other);

		/**
		 * @brief Assignment operator.
		 * @param other Location object to assign from.
		 * @return Reference to this Location.
		 */
		const Location& operator=(const Location& other);

		// ----------------- Getters -----------------
		/// @brief Get the path of this Location.
		const std::string& getLocationPath(void) const;

		/// @brief Get the upload path for this Location.
		const std::string& getLocationUploadPath(void) const;

		/// @brief Get the root directory for this Location.
		const std::string& getLocationRoot(void) const;

		/// @brief Get the index files for this Location.
		const std::vector<std::string>& getLocationIndex(void) const;

		/// @brief Get the autoindex setting for this Location.
		const int& getLocationAutoIndex(void) const;

		/// @brief Get the maximum client request size for this Location.
		const unsigned& getLocationClientMaxSize(void) const;

		/// @brief Get the allowed HTTP methods for this Location.
		const std::unordered_set<std::string>& getLocationAllowedMethods(void) const;

		/// @brief Get the allowed CGI handlers for this Location.
		const std::map<std::string, std::string>& getLocationAllowedCgi(void) const;

		/// @brief Get the return directive (status code and target).
		const std::pair<int, std::string>& getLocationReturnPages(void) const;

		/// @brief Get the error pages configured for this Location.
		const std::unordered_map<int, std::string>& getLocationErrorPages(void) const;

		// ----------------- Setters -----------------
		/// @brief Set the path for this Location.
		void setLocationPath(const std::string& path);

		/// @brief Set the upload path for this Location.
		void setLocationUploadPath(const std::string& path);

		/// @brief Set the root directory for this Location.
		void setLocationRoot(const std::string& root);

		/// @brief Set the index files for this Location.
		void setLocationIndex(const std::vector<std::string>& index);

		/// @brief Set the autoindex value for this Location.
		void setLocationAutoIndex(const int autoIndex);

		/// @brief Set the maximum client request size for this Location.
		void setLocationClientMaxSize(const unsigned maxSize);

		/// @brief Set the allowed HTTP methods for this Location.
		void setLocationAllowedMethods(const std::unordered_set<std::string>& methods);

		/// @brief Set the return directive for this Location.
		void setLocationReturnPages(const std::pair<int, std::string>& returnPages);

		/// @brief Set the error pages for this Location.
		void setLocationErrorPages(const std::unordered_map<int, std::string>& errorPages);

		/// @brief Build a return directive pair from a vector of strings.
		static std::pair<int, std::string> setLocationReturnPages(std::vector<std::string>& returnPageVector);

		// ----------------- Validators -----------------
		/// @brief Validate the allowed methods directive.
		void validateAllowedMethodsDirective(const std::vector<std::string>& allowedMethodsVector);

		/// @brief Validate the allowed CGI directive.
		void validateAllowedCgiDirective(const std::vector<std::string>& allowedCgiVector);
};

#endif
