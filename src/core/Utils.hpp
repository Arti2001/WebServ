#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <ctime>
#include <sstream>

class Utils {
    public:
        static std::string currentDateString();
        static std::string serverNameString();
};
#endif