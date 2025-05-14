#include "Server.hpp"

Server::Server(int socketFd, const vServer& vServer) : _vServer(vServer){
	this->_socketFd = socketFd;

	std::string ip = _vServer.getServerIp();

}
Server::~Server() {}

int	Server::getSocketFd(void) const {
	return (_socketFd);
}