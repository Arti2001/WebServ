/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:19 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:18:38 by vovashko      ########   odam.nl         */
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
 * @note All methods are static and can be called without instantiating the class
 */
class Utils {
    public:
        // Date and time utilities
        static std::string currentDateString();
        static std::string serverNameString();
        
        // String manipulation utilities
        static void trim(std::string& line);
        static std::string joinPaths(const std::string& path1, const std::string& path2);
};

#endif