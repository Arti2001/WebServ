#include "Utils.hpp"

std::string Utils::currentDateString() {
    std::time_t t = std::time(nullptr);
    char buffer[100];
    std::tm tm = *std::gmtime(&t);
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buffer);
}

std::string Utils::serverNameString() {
    return "webserv/1.0";
}