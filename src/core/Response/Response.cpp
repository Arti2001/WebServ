/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:05:00 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/14 15:42:55 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : {}

Response::Response(Request *request, ServerManager *ServerManager, int clientSocket) : 
    _request(request),
    _serverManager(ServerManager),
    _clientSocket(clientSocket),
    _statusCode(200),
    _statusMessage("OK")
{
    matchServer();
    matchLocation();
}

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

void matchServer() {
    const std::vector<const vServer*>&	subServConfigs = _serverManager->findServerConfigsByFd(socketClientConnectedTo);
	if (subServConfigs.empty()) {
		std::cerr << "No server configurations found for the connected socket." << std::endl;
		// Handle the error, e.g., return a 500 Internal Server Error response
		_response.setStatusCode(500);
        return ;
	}
    std::string hostHeaderValue = _request->getHeaders().at("Host");
    if (hostHeaderValue.empty()) {
        _response.setStatusCode(400);
        return ;
    }
    _serverConfig = _serverManager->findServerConfigByName(subServConfigs, hostHeaderValue);
    if (!_serverConfig) {
        _response.setStatusCode(404);
        return ;
    }
}

void matchLocation() {
    const Location* defaultLocation = _serverManager->findDefaultLocationBlock(_serverConfig.getServerLocations());
    if (!defaultLocation) {
        std::cerr << "No default location block found for the server." << std::endl;
        // Handle the error, e.g., return a 500 Internal Server Error response
        _response.setStatusCode(500);
        return ;
    }
    _location = _serverManager->findLocationBlockByUri(_serverConfig, _request->getUri());
    if (!_location) {
        _location = defaultLocation;
        std::cerr << "No matching location block found for the request URI. Using default." << std::endl;
    }
}