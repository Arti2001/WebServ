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


void Utils::trim(std::string& line) {
	size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        line.clear();
        return;
    }

    size_t end = line.find_last_not_of(" \t\r\n");
    line = line.substr(start, end - start + 1);
}