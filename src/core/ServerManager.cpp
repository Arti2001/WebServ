#include "ServerManager.hpp"

#include <filesystem>	
#include <utility>

//constructors
ServerManager::ServerManager(char* fileName, int epollSize) {
	std::string fileNameStr;

	
	if (fileName) {
		fileNameStr = fileName;
		struct stat s;
		if (stat(fileName, &s) == 0 && S_ISDIR(s.st_mode)) {
			throw ServerManagerException(fileNameStr + " is not a file.");
		}
	} else {
		fileNameStr = "default.conf";
	}
	if (std::filesystem::path(fileNameStr).extension() != ".conf")
		throw ServerManagerException("Invalid file name: use .conf extention");

	_configFileFd.open(fileNameStr);
	if (!_configFileFd) {
		throw ServerManagerException("Failed to open config file: " + fileNameStr);
	} else {
		std::cout << "File: '" << fileNameStr << "' is opened." << "\n";
	}

	_epollFd = epoll_create(epollSize);
	if (_epollFd == -1) {
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
		std::cerr << "ConfigParser::Error: " << ex.what()<< "\n";
		exit(EXIT_FAILURE);
	}
}

//void	ServerManager::createDefaultConfig(void) {

//	vServer	defaulConfig;
//	Location location (defaulConfig);

//	defaulConfig.getServerLocations().emplace("/", location);

//	_vServers.push_back(defaulConfig);
//}




const char*	ServerManager::ServerManagerException::what() const noexcept {
	
	return (_message.c_str());
}






void	ServerManager::groupServers(const std::vector<vServer>& _vServers) {
	std::unordered_set<std::string> seenServerNames;
	for (const vServer& vServer : _vServers) {
		const std::string& hostPort = vServer.getServerIpPort();
		_hostVserverMap[hostPort].push_back(&vServer);
	}
	std::cout << "Grouped servers by host and port." << "\n";
}






void	ServerManager::setServers() {
	
	std::map<std::string, std::vector<const vServer*>>::iterator	it = _hostVserverMap.begin();

	for (;it != _hostVserverMap.end(); it++) {

		const std::string& host = it->second.at(0)->getServerIp();
		const std::string& port = it->second.at(0)->getServerPort();
		int	socketFd = getSocketFd(host, port);
		_servers.emplace_back(socketFd, it->second);

	}
	std::cout << "Servers are set up and ready to run." << _servers.size() << "that many servers" << "\n";
}




int	ServerManager::getSocketFd(const std::string& host, const std::string& port) {
	
	addrinfo*	addrList = getAddrList(host, port);
	int			socketFd = bindSocket(addrList);
	
	setNonBlocking(socketFd);
	if (listen(socketFd, QUEUE_LENGTH) == -1)
	{
		close(socketFd);
		int err = errno;
		throw ServerManagerException("listen(): " + std::string(strerror(err)));
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
		throw ServerManagerException(std::string("getaddrinfo(): ") + gai_strerror(infoRet));
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
			int err = errno;
			throw ServerManagerException("setsockopt(): " + std::string(strerror(err)));
		}
		if (bind(socketFd, cpList->ai_addr, cpList->ai_addrlen) == 0) {
			std::cout<< "Successfully binded" << "\n";
			break;
		}
		close(socketFd);
		socketFd = -1;
	}
	freeaddrinfo(addrList);
	if (socketFd == -1) {
		int err = errno;
		throw ServerManagerException("socket(): " + std::string(strerror(err)));
	}
	return (socketFd);
}




void	ServerManager::setNonBlocking(int fd) {

	int err = 0;
	int prevFlags  = fcntl(fd, F_GETFL);

	if (prevFlags == -1) {
		err = errno;
		throw ServerManagerException("fctl(F_GETFL): " + std::string(strerror(err)));
	}
	if (fcntl(fd, F_SETFL, prevFlags | O_NONBLOCK) == -1) {
		err = errno;
		throw ServerManagerException("fctl( F_SETFL): " + std::string(strerror(err)));
	}
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
		std::cout << "Set listening socket fd: " << _servers[i].getSocketFd() << " to EPOLLIN event." << "\n";
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
	std::cout << "Running servers..." << "\n";
	setSocketsToEpollIn();//all listening fds are set to IN event now
	while (running) {
		int timeout = 1000;
		int readyFds = epoll_wait(_epollFd, epollEvents, EPOLL_CAPACITY, timeout);
		if (readyFds == -1) {
			int err = errno;
			throw ServerManagerException("epol_wait(): " + std::string(strerror(err)));
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

	setNonBlocking(clientFd);
	setEpollCtl(clientFd, EPOLLIN, EPOLL_CTL_ADD);
	_fdClientMap.emplace(clientFd, Client(serverFd, this));
	// _fdClientMap.emplace(std::piecewise_construct, std::forward_as_tuple(clientFd), std::forward_as_tuple(serverFd, this));

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



const std::vector<const vServer*> ServerManager::findServerConfigsByFd(int fd) const{

	for( const Server& server : _servers) {

		if (server.getSocketFd() == fd) 
		{
			std::cout << "Config for " << fd << " found." << "\n";
			return (server.getServConfigs());
		}
	}
	std::cout << "No config found for fd: " << fd << "\n";
	return (std::vector<const vServer*>());
}



const vServer* ServerManager::findServerConfigByName(const std::vector<const vServer*>& subConfigs, std::string serverName) const
{
	if (subConfigs.empty()) {
		return nullptr;
	}
	const vServer* defaultServConfig = subConfigs[0];
	for (const vServer* config : subConfigs) {

		for(const std::string& name : config->getServerNames()) {

			if (name == serverName) {
				std::cout << "Found server config for name: " << serverName << "\n";
				return(config);
			}
		}
	}
	std::cout << "No server config found for name: " << serverName << ", falling back to default config." << "\n";
	return(defaultServConfig); // fallback
}

const Location*	ServerManager::findDefaultLocationBlock(const std::map<std::string,Location>& locations) const {

	if(locations.find("/") != locations.end())
		return(&locations.at("/"));
	return(nullptr);
}

const Location*	ServerManager::findLocationBlockByUri(const vServer& serverConfig, const std::string& uri) const {
	const std::map<std::string, Location>& locations = serverConfig.getServerLocations();

	size_t longestMatchLen = 0;
	const Location* bestMatchLocation = nullptr;

	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		const std::string& locationPath = it->first;

		if (uri.find(locationPath) == 0 && locationPath.length() > longestMatchLen) {
			bestMatchLocation = &(it->second);
			longestMatchLen = locationPath.length();
		}
	}
	if (bestMatchLocation) {
		return (bestMatchLocation);
	}
	const Location* defaultLocation = findDefaultLocationBlock(locations);
	if (!defaultLocation) {
		std::cout<< "No location block found, no  default location block found " << "\n";
		return (nullptr); // better to return nullptr if no default location is found as it inicates that something went wrogn with creation of default location
		
	}
		
	std::cout<< "No  location block found, fell back to default location block " << "\n";
	return (defaultLocation);
}


void ServerManager::closeAllSockets() {

	for (const Server& server : _servers) {
		close(server.getSocketFd());
	}
}