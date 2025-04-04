/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:46:52 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/04 13:28:30 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"
/*
Plan:
    +++Store incoming data. check

    +++Check if request is complete using \r\n\r\n or Content-Length.

    Extract and parse only when full request is received.

    +++Handle multiple requests in one recv() call.

    Reject malformed requests (400 Bad Request).
*/

void RequestParser::parseBody(std::string& body, HTTPRequest& request)
{
    std::string content_type = request._headers["Content-Type"];

    if (content_type.find("application/json") != std::string::npos)
    {
        std::cout << "Got JSON" << std::endl;
        request._body = body;
    }
    else if (content_type.find("multipart/form-data") != std::string::npos)
    {   
        std::cout << "Got multipart" << std::endl;
        request._body = body;
    }
    else if (content_type.find("application/x-www-form-urlencode") != std::string::npos)
    {
        std::cout << "Got urlencoded" << std::endl;
        request._body = body;
    }
    else
    {
        request._body = body;
    }
}

std::pair<std::string, std::string> RequestParser::parseHeaders(std::string line, std::pair<std::string, std::string>& header_parsed)
{
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos)
    {
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        if (!key.empty() && std::isspace(key[0]))
        {
            throw std::runtime_error("400 BadRequest: Header name cannot start with whitespace");
        }
        key.erase(key.find_last_not_of(" \r\n\t") + 1);
        value.erase(0, value.find_first_not_of(" \r\n\t"));
        value.erase(value.find_last_not_of(" \r\n\t") + 1);

        header_parsed.first = key;
        header_parsed.second = value;
    }
    return header_parsed;
}

void RequestParser::parseFirstLineAndHeaders(std::string full_request, HTTPRequest& request)
{
    std::istringstream stream(full_request);
    std::unordered_map<std::string, std::string> headers;
    
    std::string method, path, version;
    
    stream >> method >> path >> version;
    
    request._method = method;
    request._uri = path;
    request._version = version; 

    std::string line;
    std::pair<std::string, std::string> header_parsed;
    while(std::getline(stream, line) && !line.empty())
    {
        if (line == "\r" || line == "\n")
            continue ;
        header_parsed = parseHeaders(line, header_parsed);
        headers[header_parsed.first] = header_parsed.second;
    }
    request._headers = headers;
}

std::unordered_map<int , HTTPRequest>& RequestParser::handleIncomingRequest(int fd, const std::string& raw_data, std::unordered_map<int, HTTPRequest>& resultMap)
{
    _request_buffers[fd] += raw_data;
    
    while (true)
    {
        
        std::size_t pos = _request_buffers[fd].find("\r\n\r\n");
        if (pos == std::string::npos)
        {
            return resultMap; // Request is not complete yet
        }
        std::string firstLine_and_headers = _request_buffers[fd].substr(0, pos + 4);
        
        HTTPRequest request;
        parseFirstLineAndHeaders(firstLine_and_headers, request);
        
        int content_length = 0;
        if (request._headers.find("Content-Length") != request._headers.end())
        {
            content_length = std::stoi(request._headers["Content-Length"]);
        }
        
        std::size_t total_request_size = pos + 4 + content_length;
        if (total_request_size > _request_buffers[fd].size())
        {
            return resultMap; // Request is not complete yet
        }

        std::string full_request = _request_buffers[fd].substr(0, total_request_size);
        std::string body = full_request.substr(pos + 4);
        parseBody(body, request);
        
        resultMap[fd] = request;
        
        _request_buffers[fd].erase(0, total_request_size);
        if (_request_buffers[fd].empty())
        {
            break;
        }
    }
    return resultMap;    
}