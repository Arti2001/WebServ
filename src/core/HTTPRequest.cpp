/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:30:40 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/03 16:11:38 by pminialg      ########   odam.nl         */
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