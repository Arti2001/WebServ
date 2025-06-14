/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:05:00 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/18 16:05:01 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _status_code(200), _reason_phrase("OK") {}

Response::~Response() {}

int Response::getStatusCode() const {
    return _status_code;
}

void Response::setStatusCode(int statusCode) {
    _status_code = statusCode;
}

const std::string& Response::getReasonPhrase() const {
    return _reason_phrase;
}

void Response::setReasonPhrase(const std::string& reasonPhrase) {
    _reason_phrase = reasonPhrase;
}

void Response::addHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

const std::map<std::string, std::string>& Response::getHeaders() const {
    return _headers;
}

void Response::setBody(const std::vector<char>& body) {
    _body = body;
}

void Response::setBody(std::vector<char>&& body) {
    _body = std::move(body);
}

const std::vector<char>& Response::getBody() const {
    return _body;
}

std::vector<char> Response::serialize() const {
    std::ostringstream resp_line;
    resp_line << "HTTP/1.1 " << _status_code << " " << _reason_phrase << "\r\n";
    for (auto& h : _headers) {
        resp_line << h.first << ": " << h.second << "\r\n";
    }
    resp_line << "\r\n";
    std::string resp_line_str = resp_line.str();
    std::vector<char> response(resp_line_str.begin(), resp_line_str.end());
    response.insert(response.end(), _body.begin(), _body.end());
    return response;
}