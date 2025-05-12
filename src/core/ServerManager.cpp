#include "ServerManager.hpp"

//#include <iostream>
//#include <cstring>
//#include <unistd.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
//#include <errno.h>

//void checkSocketListen(int socketFd) {
//    if (listen(socketFd, SOMAXCONN) == -1) {
//        std::cerr << "listen() failed on socket " << socketFd << ": " << strerror(errno) << "\n";

//        switch (errno) {
//            case EADDRINUSE:
//                std::cerr << "Address already in use. Another server may be using this port.\n";
//                break;
//            case EBADF:
//                std::cerr << "Invalid socket file descriptor.\n";
//                break;
//            case ENOTSOCK:
//                std::cerr << "The file descriptor is not a socket.\n";
//                break;
//            case EOPNOTSUPP:
//                std::cerr << "Socket type does not support listen().\n";
//                break;
//            case EINVAL:
//                std::cerr << "Socket is not bound, or already listening.\n";
//                break;
//            default:
//                std::cerr << "Unknown listen() error.\n";
//        }

//        close(socketFd); // optional cleanup
//    } else {
//        std::cout << "Socket is now listening (fd: " << socketFd << ")\n";
//    }
//}

//constructors
ServerManager::ServerManager(std::string& fileName, int epollSize) {

	if (fileName.empty()){
		fileName = DEFAULT_CONFIG_FILE_PATH;
	}
	_configFileFd.open(fileName);
	if (!_configFileFd) {
		throw (ServerManagerException("Failed to open " + fileName));
	}
	else{
		std::cout << "File: '" << fileName << "' is opend." << "\n";
	}

	_epollFd = epoll_create(epollSize);
	if (_epollFd == -1){
		throw ServerManagerException("Failed to create an epoll instance.");
	}
}



Client::Client() {
	this->clientBytesSent = 0;
	this->cleintResponse = "Default response";
}

ServerManager::ServerManagerException::ServerManagerException(const std::string& msg) : _message(msg) {

}





//Destructor
ServerManager::~ServerManager() {

	if (_configFileFd.is_open()) {
		_configFileFd.close();
	}
	close(_epollFd);
}

//getters
std::ifstream&	ServerManager::getConfigFileFd( void ) {
	
	return (_configFileFd);
}

int	ServerManager::getEpollFd( void ) const {
	
	return (_epollFd);
}

std::vector<vServer>&		ServerManager::getvServers( void )
{

	return (_vServers);
}





//methods

const char*	ServerManager::ServerManagerException::what() const noexcept {
	
	return (_message.c_str());
}

int	ServerManager::getSocketFd( const vServer& vServer) {
	
	addrinfo*	addrList = getAddrList(vServer);
	int			socketFd = bindSocket(addrList);

	if (!setNonBlocking(socketFd))
		throw ServerManagerException("Failed to set a file descriptor to a non-blocking mode");
	if (listen(socketFd, QUEUE_LENGTH) == -1)
	{
		close(socketFd);
		throw ServerManagerException("Failed to  listen on a file descriptor");
	}
	return (socketFd);
}






bool	ServerManager::setNonBlocking(int fd) {

	int prevFlags  = fcntl(fd, F_GETFL);

	if (prevFlags == -1) {
		std::cerr << " Error: fctl(): Failed to get the fd's flags!"<< "\n";
		return (false);
	}
	if (fcntl(fd, F_SETFL, prevFlags | O_NONBLOCK) == -1) {
		std::cerr << " Error: fctl(): Failed to set the fd's flags!"<< "\n";
		return (false);
	}
	return (true);
}





void	ServerManager::parsConfigFile(std::vector<vServer>& _vServers) {
	
	ParseConfig									parser;
	std::map<size_t, std::vector<std::string>>	roughData;
	
	try{
		roughData = parser.prepToTokenizeConfigData(getConfigFileFd());
		parser.tokenizeConfigData(roughData);
		parser.parsConfigFileTokens(_vServers);
	}catch(ParseConfig::ConfException& ex){
		std::cerr << "Error: " << ex.what()<< "\n";
		exit(EXIT_FAILURE);
	}
}





addrinfo* ServerManager::getAddrList(const vServer& vServer) {

	struct addrinfo	hints, *addrList;
	int				infoRet;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	
	const std::string& host = vServer.getServerIp();
	const std::string& port = vServer.getServerPort();

	infoRet = getaddrinfo(host.c_str(), port.c_str(), &hints, &addrList);
	if (infoRet != 0 ) {
		throw ServerManagerException(std::string("getaddrinfo failed: ") + gai_strerror(infoRet));
	}
	return (addrList);
}





int	ServerManager::bindSocket(addrinfo* addrList) {

	addrinfo*	cpList;
	int			socketFd = -1;

	for (cpList = addrList; cpList != NULL; cpList = cpList->ai_next) {

		socketFd = socket(cpList->ai_family, cpList->ai_socktype, cpList->ai_protocol);
		if (socketFd == -1) {
			continue;
		}
	
		if (bind(socketFd, cpList->ai_addr, cpList->ai_addrlen) == 0) {
			std::cerr << "Successfully binded" << "\n";
			break;
		}
		close(socketFd);
		socketFd = -1;
	}
	freeaddrinfo(addrList);
	if (socketFd == -1) {
		throw ServerManagerException("Failed to bind socket to any address");
	}
	return (socketFd);
}




void	ServerManager::setEpollCtl( int targetFd, int eventFlag, int operation){

	struct epoll_event	targetEvent;

	targetEvent.data.fd = targetFd;
	targetEvent.events = eventFlag;

	if (epoll_ctl(_epollFd, operation, targetFd, &targetEvent) == -1) {

		throw ServerManagerException("epoll_ctl failed");
	}
}




void	ServerManager::setServers(const std::vector<vServer>& vServers) {

	for (const vServer& vServer : vServers) {

		int	socketFd = getSocketFd(vServer);
		_servers.emplace_back(socketFd, vServer);
	}
}





void	ServerManager::setSocketsToEpollIn(void) {

	for (size_t i = 0; i < _servers.size(); i++)
	{
		setEpollCtl(_servers[i].getSocketFd(), IN, EPOLL_CTL_ADD);
	}
}





void	ServerManager::runServers(void) {

	struct epoll_event	epollEvents[EPOLL_CAPACITY];
	setSocketsToEpollIn();//all listening fds are set to IN event now

	while (1) {
		int readyFds = epoll_wait(_epollFd, epollEvents, EPOLL_CAPACITY, 0);
		if (readyFds == -1)
			throw ServerManagerException("epoll_wait() failed");
		manageEpollEvent(epollEvents, readyFds);
	}
}




bool ServerManager::isListeningSocket(int fd) {

	for(const Server& server : _servers) {

		if (server.getSocketFd() == fd) {
			return (true);
		}
	}
	return (false);
}

void	ServerManager::manageEpollEvent(const struct epoll_event* currEvent, int readyFds) {

	int						acceptedSocket;
	struct sockaddr_storage	clientAddr;
	socklen_t				clientAddrLen = sizeof(clientAddr);

	for (int i = 0; i < readyFds; i++) {

		if (isListeningSocket(currEvent->data.fd)) {
	
			acceptedSocket = accept(currEvent->data.fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
			if (acceptedSocket == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					std::cerr << "No client connections available at the moment." << std::endl;
					continue;
				}
				throw ServerManagerException("Failed to accept the client socket");
			}
			if (!setNonBlocking(acceptedSocket)) {
				close(acceptedSocket);
				throw ServerManagerException("Failed to set the acceptedSocket to a NON-BLOCKING mode.");
			}
			setEpollCtl(acceptedSocket, IN, EPOLL_CTL_ADD);
			Client	client;
			_fdClientDataMap[acceptedSocket] = client;
		
		}
		else if (currEvent->events & EPOLLIN) {
			readRequest(acceptedSocket);

		}
		else if (currEvent->events & EPOLLOUT) {

			sendResponse(acceptedSocket);
		}
	}
}






























void	ServerManager::readRequest (int clientFd) {

	char		recBuff[RECBUFF];//8KB
	ssize_t		bytesRead;

	bytesRead = recv(clientFd, recBuff, RECBUFF - 1, 0);

	if (bytesRead > 0)
	{
		recBuff[bytesRead] = '\0';
		std::cout << "######Request#####\n" << recBuff << "\n";

		//prepResponse(clientFd);
		setEpollCtl(clientFd, OUT, EPOLL_CTL_MOD);
		return ;
	}
	else if (bytesRead == 0){
		std::cout << "Connection lost" << bytesRead << "\n";
		close(clientFd);
		return;
	}
	else
		std::cout << "Nod data to read !"<< "\n";
}

void	ServerManager::sendResponse(int clientFd) {

	size_t&		totalBytesSent = _fdClientDataMap[clientFd].clientBytesSent;
	const char*	servResp = _fdClientDataMap[clientFd].cleintResponse.c_str();
	ssize_t		bytesSent;
	size_t		responseSize = strlen(servResp);

	
	bytesSent = send(clientFd, servResp + totalBytesSent, responseSize - totalBytesSent, 0);
	if (bytesSent == -1) {
		setEpollCtl(clientFd, OUT, EPOLL_CTL_MOD);
		return;
	}
	totalBytesSent += bytesSent;
	if (totalBytesSent == strlen(servResp)) {
		std::cout << "All data sent: Connection closed" << "\n";
		setEpollCtl(clientFd, IN, EPOLL_CTL_MOD);
		totalBytesSent = 0;

		close(clientFd);
		_fdClientDataMap.erase(clientFd);
	}
}


//void	ServerManager::prepResponse(int clientFd) {

//	_clients[clientFd].response = "lol";
//	_clients[clientFd].bytesSent = 0;
//}