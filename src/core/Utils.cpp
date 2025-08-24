/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:21 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:18:27 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

/**
 * @brief Generates the current date and time in HTTP format
 * @return String containing the current date in RFC 1123 format (e.g., "Mon, 25 Aug 2025 14:30:00 GMT")
 * @note Uses GMT timezone for consistency with HTTP standards
 */
std::string Utils::currentDateString() {
    std::time_t t = std::time(nullptr);
    char buffer[100];
    std::tm tm = *std::gmtime(&t);
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buffer);
}

/**
 * @brief Returns the server identification string
 * @return String containing the server name and version
 * @note Used in HTTP headers like Server and User-Agent
 */
std::string Utils::serverNameString() {
    return "webserv/1.0";
}

/**
 * @brief Removes leading and trailing whitespace from a string
 * @param line The string to trim (modified in place)
 * @return None
 * @note Removes spaces, tabs, carriage returns, and newlines from both ends
 */
void Utils::trim(std::string& line) {
	size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        line.clear();
        return;
    }

    size_t end = line.find_last_not_of(" \t\r\n");
    line = line.substr(start, end - start + 1);
}

/**
 * @brief Joins two path components, handling directory separators correctly
 * @param path1 The first path component
 * @param path2 The second path component
 * @return The joined path string
 * @note Automatically handles cases where one or both paths end/begin with directory separators
 */
std::string Utils::joinPaths(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    
    if (path1.back() == '/' && path2.front() == '/') {
        return path1 + path2.substr(1);
    } else if (path1.back() == '/' || path2.front() == '/') {
        return path1 + path2;
    } else {
        return path1 + "/" + path2;
    }
}