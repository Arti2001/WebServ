#include "Server.hpp"

Server::Server(int socketFd, std::vector<const vServer*> vServers) : _vServers(vServers){
	this->_socketFd = socketFd;
	

	std::cout << _vServers.at(0)->getServerIp()<< "\n";

}
Server::~Server() {}

int	Server::getSocketFd(void) const {
	return (_socketFd);
}

const std::vector<vServer>& Server::getServConfigs( void ) const {

	return(_servConfigs);
}