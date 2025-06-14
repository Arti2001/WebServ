#include "ServerManager.hpp"
#include <tuple>
#include <utility>

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

std::vector<vServer>&		ServerManager::getVirtualServers( void )
{
	
	return (_vServers);
}

std::vector<Server>&	ServerManager::getServers(void) {
	return (_servers);
}





//methods

void	ServerManager::parsConfigFile(std::vector<vServer>& _vServers) {
	
	ParseConfig									parser;
	std::map<size_t, std::vector<std::string>>	roughData;
	
	try{
		roughData = parser.prepToTokenizeConfigData(getConfigFileFd());
		parser.tokenizeConfigData(roughData);
		parser.parseConfigFileTokens(_vServers);
		
	}catch(ParseConfig::ConfException& ex){
		std::cerr << "Error: " << ex.what()<< "\n";
		exit(EXIT_FAILURE);
	}
}




const char*	ServerManager::ServerManagerException::what() const noexcept {
	
	return (_message.c_str());
}






void	ServerManager::groupServers(const std::vector<vServer>& _vServers) {

	for (const vServer& vServer : _vServers) {
		const std::string& hostPort = vServer.getServerIpPort();
		_hostSetMap[hostPort].push_back(&vServer);
	}
}






void	ServerManager::setServers() {
	
	std::map<std::string, std::vector<const vServer*>>::iterator	it = _hostSetMap.begin();

	for (;it != _hostSetMap.end(); it++) {

		const std::string& host = it->second.at(0)->getServerIp();
		const std::string& port = it->second.at(0)->getServerPort();
		int	socketFd = getSocketFd(host, port);
		_servers.emplace_back(socketFd, it->second);

	}
}




int	ServerManager::getSocketFd(const std::string& host, const std::string& port) {
	
	addrinfo*	addrList = getAddrList(host, port);
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




addrinfo* ServerManager::getAddrList(const std::string& currHost, const std::string& currPort) {

	struct addrinfo	hints, *addrList;
	int				infoRet;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	
	infoRet = getaddrinfo(currHost.c_str(), currPort.c_str(), &hints, &addrList);
	if (infoRet != 0 ) {
		throw ServerManagerException(std::string("getaddrinfo failed: ") + gai_strerror(infoRet));
	}
	return (addrList);
}


std::map<int, Client>&	ServerManager::getFdClientMap(void) {

	return (_fdClientMap);
}


int	ServerManager::bindSocket(addrinfo* addrList) {

	addrinfo*	cpList;
	int			socketFd(-1);

	for (cpList = addrList; cpList != NULL; cpList = cpList->ai_next) {

		socketFd = socket(cpList->ai_family, cpList->ai_socktype, cpList->ai_protocol);
		if (socketFd == -1) {
			continue;
		}
		int _switch = ENABLE;
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &_switch, sizeof(_switch)) == -1){
			close(socketFd);
			throw ServerManagerException("Failed to set socket.");
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




void	ServerManager::setEpollCtl( int targetFd, int eventFlag, int operation){

	struct epoll_event	targetEvent;

	targetEvent.data.fd = targetFd;
	targetEvent.events = eventFlag;
	if (epoll_ctl(_epollFd, operation, targetFd, &targetEvent) == -1) {

		close(targetFd);
	}
}




void	ServerManager::setSocketsToEpollIn(void) {

	for (size_t i = 0; i < _servers.size(); i++)
	{
		setEpollCtl(_servers[i].getSocketFd(), EPOLLIN, EPOLL_CTL_ADD);
	}
}



void	ServerManager::closeClientFd(int clientFd){

	epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
	//_fdClientMap.at(clientFd).setIsClosed(true);
	close(clientFd);
	_fdClientMap.erase(clientFd);
}

//void	ServerManager::closeIdleConnections() {

//	time_t	currTime = std::time(nullptr);

//	for(std::map<int, Client>::iterator it = _fdClientMap.begin(); it != _fdClientMap.end(); it++) {
//		if ((currTime - it->second.getLastActiveTime()) > SERVER_TIMEOUT_MS) {
//			std::cout<< "Found an idle connection"<< "\n";
//			const std::vector<const vServer*>& fromThisServer = findServerConfigsByFd(it->second.getServerFd());
//			std::cerr<< "Time out: Closing the client hosted at the host: " + fromThisServer.at(0)->getServerIpPort()<< "\n";
//			closeClientFd(it->first);
//		}
//	}
//}


//helper functions

bool	ServerManager::isListeningSocket(int fd) {
	for(const Server& server : _servers) {
		
		if (server.getSocketFd() == fd) {
			return (true);
		}
	}
	return (false);
}


bool	ServerManager::isClientSocket(int fd) {

	std::map<int, Client>::iterator it = _fdClientMap.begin();

	for (;it != _fdClientMap.end(); it++) {

		if (it->first == fd)
			return (true);
	}
	return (false);
}




void	ServerManager::runServers(void) {

	struct epoll_event	epollEvents[EPOLL_CAPACITY];

	setSocketsToEpollIn();//all listening fds are set to IN event now
	while (running) {
		int timeout = 1000;
		int readyFds = epoll_wait(_epollFd, epollEvents, EPOLL_CAPACITY, timeout);
		//if (readyFds == 0)
		//{
		//	closeIdleConnections();
		//	continue;
		//}
		if (readyFds == -1) {
			throw ServerManagerException("epoll_wait() failed");
		}
		for (int i = 0; i < readyFds; i++) {
			manageEpollEvent(epollEvents[i]);
		}
	}
}




void	ServerManager::manageListenSocketEvent(const struct epoll_event& currEvent) {
	struct sockaddr_storage	clientAddr;
	socklen_t				clientAddrLen = sizeof(clientAddr);
	
	int acceptedSocket = accept(currEvent.data.fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (acceptedSocket == -1) {
		
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			std::cerr << "No client connections available at the moment." << std::endl;
			return;
		}
		throw ServerManagerException("Failed to accept the client socket");
	}
	addClientToMap(acceptedSocket, currEvent.data.fd);
	
}




void ServerManager::addClientToMap(int clientFd, int serverFd) {
	//callback function

	if (!setNonBlocking(clientFd)) {
		
		close(clientFd);
		throw ServerManagerException("Failed to set the acceptedSocket to a NON-BLOCKING mode.");
	}
	setEpollCtl(clientFd, EPOLLIN, EPOLL_CTL_ADD);
	// _fdClientMap.emplace(clientFd, Client(serverFd, this));
	_fdClientMap.emplace(std::piecewise_construct, std::forward_as_tuple(clientFd), std::forward_as_tuple(serverFd, this));

}




void	ServerManager::manageEpollEvent(const struct epoll_event& currEvent) {

	int	fd = currEvent.data.fd;

	if (isListeningSocket(fd)) {
		manageListenSocketEvent(currEvent);
	}
	else if ((currEvent.events & EPOLLIN) && isClientSocket(fd)) {
		_fdClientMap.at(fd).handleRequest(fd);
	}
	else if ((currEvent.events & EPOLLOUT) && isClientSocket(fd)) {
		_fdClientMap.at(fd).handleResponse(fd);
	}
	
}



const std::vector<const vServer*>& ServerManager::findServerConfigsByFd(int fd) {

	std::cout<<"here"<<"\n";
	for( const Server& server : _servers) {

		if (server.getSocketFd() == fd) 
			return (server.getServConfigs());
	}
	return (std::vector<const vServer*>());
}



const vServer& ServerManager::findServerConfigByName(const std::vector<const vServer*>& subConfigs, std::string serverName) {

	const vServer* defaultServConfig = subConfigs.at(0);
	for (const vServer* config : subConfigs) {

		for(const std::string& name : config->getServerNames()) {

			if (name == serverName) {
				return(*config);
			}
		}
	}
	return(*defaultServConfig);
}

const Location*	ServerManager::findDefaultLocationBlock(const std::vector<Location>& locations) {

	for(const Location& loc : locations) {
		if (loc._locationPath == "/") {
			return (&loc);
		}
	}
	return(nullptr);

}

const Location	ServerManager::findLocationBlockByUri(const vServer& serverConfig, const std::string& uri) {

	const std::vector<Location>& locations = serverConfig.getServerLocations();
	size_t longestMatchLen = 0;
	const Location*	bestMatchLocation = nullptr;

	for (const Location& loc : locations) {
		
		const std::string& locationPath = loc._locationPath;
		
		if (uri.find(locationPath) == 0 && locationPath.length() > longestMatchLen ) {
			bestMatchLocation = &loc;
			longestMatchLen = locationPath.length();
		}
	}
	if (bestMatchLocation) {
		return (*bestMatchLocation);
	}

		const Location* defaultLocation = findDefaultLocationBlock(locations);
		if (!defaultLocation) {
			std::cout<< "No location block found, no  default location block found " << "\n";
			Location newLocation(serverConfig);
			return(newLocation);
		}
		
		std::cout<< "No  location block found, fell back to default location block " << "\n";
	return (*defaultLocation);
}


bool ServerManager::isDefaultLocationExist(const std::vector<Location>& locations) {

	if (locations.empty())
		throw ServerManagerException("A configuration file must have atleast a default location block");
	return (true);
}

void ServerManager::closeAllSockets() {

	for (const Server& server : _servers) {
		close(server.getSocketFd());
	}
}