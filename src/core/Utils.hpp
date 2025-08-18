/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:19 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/07/10 18:50:21 by vshkonda      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <ctime>
#include <sstream>
#include <vector>

class Utils {
    public:
        static std::string currentDateString();
        static std::string serverNameString();
        static void trim(std::string& line);
		static std::string joinPaths(const std::string& path1, const std::string& path2);
};
#endif