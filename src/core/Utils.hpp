/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:19 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/07/06 13:08:20 by vshkonda      ########   odam.nl         */
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
};
#endif