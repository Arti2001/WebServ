/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   StaticHandler.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: amysiv <amysiv@student.42.fr>                +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:05:25 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/14 14:08:04 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATICHANDLER_HPP
#define STATICHANDLER_HPP

#include "../parsingConfFile/vServer.hpp"
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "MimeTypes.hpp"
#include "../Utils.hpp"

class StaticHandler {
    private:
        Response loadErrorPage(const Location& loc, int code);
        std::vector<char> generateDirectoryListing(const std::string& fsPath, const std::string& urlPath);
        Response serveGet(const Request& req, const Location& loc);
		
		public:
        StaticHandler();
        ~StaticHandler();
		
		static	Response loadNotFound();
        Response serve(const Request& req, const Location& loc);

};

#endif