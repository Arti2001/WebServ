#include "ServerManager.hpp"

//constructors
ServerManager::ServerManager(std::string& fileName, int epollSize) {

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
std::ifstream	ServerManager::getConfigFileFd( void ) const {
	
	return (_configFileFd);
}


int	ServerManager::getEpollFd( void ) const {
	
	return (_epollFd);
}

std::vector<int> ServerManager::getSocketFds( void ) const {
	
	return (_socketFds);
}


std::vector<vServer>&		ServerManager::getVServers( void )
{

	return (_vServers);
}




//methods

const char*	ServerManager::ServerManagerException::what() const noexcept {
	
	return (_message.c_str());
}

void	ServerManager::setSocketFds( size_t currVServer  ) {
	
	addrinfo*	addrList = getAddrList(currVServer);
	int			socketFd = bindSocket(addrList);


	if (!setNonBlocking(socketFd))
		throw ServerManagerException("Failed to set a file descriptor to a non-blocking mode for the " + std::to_string(currVServer) + " Server");
	
	if (!listen(socketFd, QUEUE_LENGTH))
	{
		close(socketFd);
		throw ServerManagerException("Failed to star listening on the " + std::to_string(currVServer) + " Server");
	}
	_socketFds.push_back(socketFd);
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
	}
}


addrinfo* ServerManager::getAddrList( size_t currVServer ) {

	struct addrinfo	hints, *addrList;
	int				infoRet;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	
	const std::string& host = _vServers[currVServer].getServerIp();
	const std::string& port = _vServers[currVServer].getServerPort();

	infoRet = getaddrinfo(host.c_str(), port.c_str(), &hints, &addrList);
	if (infoRet != 0 ) {
		throw ServerManagerException(std::string("getaddrinfo failed: ") + gai_strerror(infoRet));
	}
	return (addrList);
}



int	ServerManager::bindSocket(addrinfo* addrList) {

	addrinfo*	cpList;
	int			socketFd;

	for (cpList = addrList; cpList != NULL; cpList = cpList->ai_next) {

		socketFd = socket(cpList->ai_family, cpList->ai_socktype, cpList->ai_protocol);
		if (socketFd == -1) {
			continue;
		}
		if (bind(socketFd, cpList->ai_addr, cpList->ai_addrlen) == 0) {
			std::cerr << "Successfully binded" << "\n";
			break;
		}
		close (socketFd);
	}
	if (socketFd == -1) {
		throw ServerManagerException("Failed to bind socket on any address");
	}
	freeaddrinfo(addrList);
	return (socketFd);
}