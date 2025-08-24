/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:19 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:40:16 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <ctime>
#include <sstream>
#include <vector>

/**
 * @brief Utility class providing common helper functions for the webserv application
 * @details This class contains static utility functions for common operations like date formatting,
 *          string manipulation, path handling, and server identification. All methods are static
 *          and can be called without instantiating the class.
 * @note Designed as a utility class with no state, providing pure functions for common tasks.
 */
class Utils {
    public:
        // Date and time utilities
        /**
         * @brief Generates the current date and time in HTTP format
         * @return String containing the current date in RFC 1123 format (e.g., "Mon, 25 Aug 2025 14:30:00 GMT")
         * @note Uses GMT timezone for consistency with HTTP standards
         */
        static std::string currentDateString();

        /**
         * @brief Returns the server identification string
         * @return String containing the server name and version
         * @note Used in HTTP headers like Server and User-Agent
         */
        static std::string serverNameString();
        
        // String manipulation utilities
        /**
         * @brief Removes leading and trailing whitespace from a string
         * @param line The string to trim (modified in place)
         * @return None
         * @note Removes spaces, tabs, carriage returns, and newlines from both ends
         */
        static void trim(std::string& line);

        /**
         * @brief Joins two path components, handling directory separators correctly
         * @param path1 The first path component
         * @param path2 The second path component
         * @return The joined path string
         * @note Automatically handles cases where one or both paths end/begin with directory separators
         */
        static std::string joinPaths(const std::string& path1, const std::string& path2);
};

#endif