/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:30:21 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/03 16:18:35 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <iostream>
#include <sstream>
#include <unordered_map>
#include "HTTPRequest.hpp"

class HTTPRequest;

class RequestParser
{
    private:
        std::unordered_map<int, std::string> _request_buffers;
    public:
        std::unordered_map<int , HTTPRequest>& handleIncomingRequest(int fd, const std::string& raw_data, std::unordered_map<int, HTTPRequest>& resultMap);
        void parseFirstLineAndHeaders(std::string full_request, HTTPRequest& request);
        std::pair<std::string, std::string> parseHeaders(std::string line, std::pair<std::string, std::string>& header_parsed);
        void parseBody(std::string& body, HTTPRequest& request);
};

#endif