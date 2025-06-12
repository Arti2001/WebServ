#include "ListenConfig.hpp"

ListenConfig::ListenConfig() {}

ListenConfig::ListenConfig(const ListenConfig &src) : _host(src._host), _port(src._port), _listenLine(src._listenLine) {}

ListenConfig::ListenConfig(std::string &token)
{
    std::string host;
    std::string port;
    
    size_t posColon = token.find(':');
    if (posColon!= std::string::npos) {
        host = token.substr(0, posColon);
        port = token.substr(posColon + 1);
    } else {
        size_t posDot = token.find('.');
        if (posDot != std::string::npos) {
            host = token;
            port = "80"; // Default port
        } else {
            host = "localhost";
            port = token;
        }
    }
    if (verifyHost(host)) {
        _host = host;
    } else {
        std::cerr << "Invalid host: " << host << std::endl;
        exit(1);
    }
    
    try {
        _port = std::stoi(port);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid port: " << port << std::endl;
        exit(1);
    } catch (const std::out_of_range &e) {
        std::cerr << "Port out of range: " << port << std::endl;
        exit(1);
    }

    if (!verifyPort(_port)) {
        std::cerr << "Invalid port: " << port << std::endl;
        exit(1);
    }

    _listenLine = host + ":" + port;
}

ListenConfig &ListenConfig::operator=(const ListenConfig &src) {
    if (this != &src) {
        _host = src._host;
        _port = src._port;
        _listenLine = src._listenLine;
    }
    return *this;
}

ListenConfig::~ListenConfig() {}
const std::string &ListenConfig::getHost() const {
    return _host;
}
const int &ListenConfig::getPort() const {
    return _port;
}
bool ListenConfig::verifyHost(const std::string &host) const {
    if (host == "localhost") {
        return true; // localhost is always valid
    }
    std::vector<std::string> octets = split(host, '.');
    if (octets.size() != 4){
        return false; // Invalid number of octets
    }
    for (const std::string &octet : octets) {
        if (octet.empty() || octet.size() > 3 || std::stoi(octet) < 0 || std::stoi(octet) > 255) {
            return false; // Invalid octet
        }
    }
    return true; // Valid host
}

bool ListenConfig::verifyPort(int port) const {
    return (port > 0 && port <= 65535); // Valid port range
}
void ListenConfig::setHost(const std::string &host) {
    if (verifyHost(host)) {
        _host = host;
    } else {
       std::cerr << "Invalid host: " << host << std::endl;
        exit(1);
    }
}
void ListenConfig::addPort(int port) {
    if (verifyPort(port)) {
        _port = port;
    } else {
        std::cerr << "Invalid port: " << port << std::endl;
        exit(1);
    }
}
std::string ListenConfig::getListenLine() const {
    return _listenLine;
}
void ListenConfig::setListenLine(const std::string &listenLine) {
    _listenLine = listenLine;
}

std::vector<std::string> ListenConfig::split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}