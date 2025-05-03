/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:30:25 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/24 13:22:20 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <map>
#include "RequestParser.hpp"

class HTTPRequest
{
    private:
        std::string _method;
        std::string _uri;
        std::string _version;
        std::unordered_map<std::string, std::string> _headers;
        std::string _body;
        std::unordered_map<std::string, std::string> _form_data;
        std::unordered_map<std::string, std::string> _files;

    public:
        HTTPRequest();
        ~HTTPRequest();
        std::string getMethod() const;
        std::string getUri() const;
        std::string getVersion() const;
        std::unordered_map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        void setMethod(const std::string& method);
        void setUri(const std::string& uri);
        void setVersion(const std::string& version);
        
        friend class RequestParser;

        friend std::ostream& operator<<(std::ostream& os, const HTTPRequest& req) {
            os << "Method: " << req._method << "\n";
            os << "URI: " << req._uri << "\n";
            os << "Version: " << req._version << "\n";
            os << "Headers:\n";
            for (const auto& header : req._headers) {
                os << "  " << header.first << ": " << header.second << "\n";
            }
            os << "Body: " << req._body << "\n";
            os << "Form Data:\n";
            for (const auto& header : req._form_data) {
                os << "  " << header.first << ": " << header.second << "\n";
            }
            os << "Files:\n";
            for (const auto& header : req._files) {
                os << "  " << header.first << ": " << header.second << "\n";
            }
            return os;
        }
};

#endif