/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:30:40 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/06 16:42:24 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() {}

HTTPRequest::~HTTPRequest() {}

std::string HTTPRequest::getMethod() const {return _method;}

std::string HTTPRequest::getUri() const {return _uri;}

std::string HTTPRequest::getVersion() const {return _version;}

std::unordered_map<std::string, std::string> HTTPRequest::getHeaders() const {return _headers;}

std::string HTTPRequest::getBody() const {return _body;}

std::string HTTPRequest::getRawBody() const {return _raw_body;}

void HTTPRequest::setMethod(const std::string& method) {
    _method = method;
}
void HTTPRequest::setUri(const std::string& uri) {
    _uri = uri;
}
void HTTPRequest::setVersion(const std::string& version) {
    _version = version;
}