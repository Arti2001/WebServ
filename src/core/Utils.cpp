/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: vshkonda <vshkonda@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/07/06 13:08:19 by vshkonda      #+#    #+#                 */
/*   Updated: 2025/08/24 21:44:42 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

std::string Utils::currentDateString() {
    std::time_t now = std::time(nullptr);
    std::tm* gmt = std::gmtime(&now);
    
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    
    return std::string(buffer);
}

std::string Utils::serverNameString() {
    return "webserv/1.0";
}

void Utils::trim(std::string& line) {
    if (line.empty()) return;
    
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        line.clear();
        return;
    }
    
    size_t end = line.find_last_not_of(" \t\r\n");
    line = line.substr(start, end - start + 1);
}

std::string Utils::joinPaths(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    
    bool path1EndsWithSlash = path1.back() == '/';
    bool path2StartsWithSlash = path2.front() == '/';
    
    if (path1EndsWithSlash && path2StartsWithSlash) {
        return path1 + path2.substr(1);
    } else if (path1EndsWithSlash || path2StartsWithSlash) {
        return path1 + path2;
    } else {
        return path1 + "/" + path2;
    }
}