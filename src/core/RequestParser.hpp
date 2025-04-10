/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:30:21 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/10 15:45:53 by pminialg      ########   odam.nl         */
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
        std::string urlDecode(const std::string& encoded);
        std::string extractboundary(const std::string& content_type);
        int parseMultipartFormData(const std::string& body, const std::string& boundary, 
                                    std::unordered_map<std::string, std::string>& form_data, 
                                    std::unordered_map<std::string, std::string>& files);
        void parseUrlEncodedForm(const std::string& body, std::unordered_map<std::string, std::string>& form_data);
        std::pair<std::string, std::string> parseContentDisposition(const std::string& headers_text);
        std::string extractContentDispositionParameterValue(const std::string& text, const std::string& parameter, size_t start_pos);
        
    public:
        std::unordered_map<int , HTTPRequest>& handleIncomingRequest(int fd, const std::string& raw_data, std::unordered_map<int, HTTPRequest>& resultMap);
        void parseFirstLineAndHeaders(std::string full_request, HTTPRequest& request);
        std::pair<std::string, std::string> parseHeaders(std::string line, std::pair<std::string, std::string>& header_parsed);
        void parseBody(std::string& body, HTTPRequest& request);
};

#endif