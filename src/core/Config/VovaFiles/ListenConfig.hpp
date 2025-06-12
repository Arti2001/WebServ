#include "Webserv.hpp"

class ListenConfig {
    private:
        std::string _host; // Host for the server, only handle IPv4 and localhost
        int _port; // Port for the server
        std::string _listenLine; // Listen line for the server
    public:
        ListenConfig();
        ListenConfig(std::string &token);
        ListenConfig(const ListenConfig &src);
        ListenConfig &operator=(const ListenConfig &src);
        ~ListenConfig();

        const std::string &getHost() const;
        const int &getPort() const;
        std::string getListenLine() const;
        void setListenLine(const std::string &listenLine);
        bool verifyHost(const std::string &host) const;
        bool verifyPort(int port) const;
        void setHost(const std::string &host);
        void addPort(int port);
        static std::vector<std::string> split(const std::string &s, char delimiter);
};