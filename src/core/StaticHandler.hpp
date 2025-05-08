/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 16:05:25 by pminialg          #+#    #+#             */
/*   Updated: 2025/05/08 09:53:56 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATICHANDLER_HPP
#define STATICHANDLER_HPP

#include "parsing/vServer.hpp"
#include "HTTPRequest.hpp"
#include "Response.hpp"
#include "MimeTypes.hpp"
#include "Utils.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sstream>

//struct Location {
//   std::string _path;
//   std::string _root;
//   std::string _index;
//   bool _auto_index;
//   unsigned _clientMaxSize;
//   std::vector<std::string> _allowedMethods;
//   std::unordered_map<int, std::string> _errorPages;
//};

class StaticHandler {
    private:
        Response loadErrorPage(const Location& loc, int code);
        std::vector<char> generateDirectoryListing(const std::string& fsPath, const std::string& urlPath);
        Response serveGet(const HTTPRequest& req, const Location& loc);
    
    public:
        StaticHandler();
        ~StaticHandler();

        Response serve(const HTTPRequest& req, const Location& loc);

};

#endif