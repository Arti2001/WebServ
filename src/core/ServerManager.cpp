#include "ServerManager.hpp"

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
	this->clientResponse = "Default response";
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





//methods

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




const char*	ServerManager::ServerManagerException::what() const noexcept {
	
	return (_message.c_str());
}




void	ServerManager::setServers(const std::vector<vServer>& vServers) {
	
	for (const vServer& vServer : vServers) {
		
		int	socketFd = getSocketFd(vServer);
		_servers.emplace_back(socketFd, vServer);
	}
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
	close(clientFd);
	_fdClientDataMap.erase(clientFd);
}

void	ServerManager::closeIdleConnections(int socketFd) {
	
	time_t	currTime = std::time(nullptr);
	std::map<int, Client>::iterator it = _fdClientDataMap.begin();
	std::map<int, Client>::iterator end = _fdClientDataMap.end();
	
	for(; it != end; it++) {
		
		if ((currTime - it->second.lastActiveTime) > SERVER_TIMEOUT) {

			closeClientFd(it->first);
		}
		it++;
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




void	ServerManager::runServers(void) {

	struct epoll_event	epollEvents[EPOLL_CAPACITY];

	setSocketsToEpollIn();//all listening fds are set to IN event now
	while (1) {
		int timeout = 1000;
		int readyFds = epoll_wait(_epollFd, epollEvents, EPOLL_CAPACITY, timeout);
		if (readyFds == NONE)
		{
			std::cout<< " idle connection "<<"\n";
			continue;
		}
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
	addClient(acceptedSocket);
}




void ServerManager::addClient(int clientFd) {
	
	Client	client;
	time_t	timeStapm = std::time(nullptr);
	
	if (!setNonBlocking(clientFd)) {
		
		close(clientFd);
		throw ServerManagerException("Failed to set the acceptedSocket to a NON-BLOCKING mode.");
	}
	setEpollCtl(clientFd, EPOLLIN, EPOLL_CTL_ADD);
	
	_fdClientDataMap[clientFd] = client;
	_fdClientDataMap[clientFd].lastActiveTime = timeStapm;
}




void	ServerManager::manageEpollEvent(const struct epoll_event& currEvent) {
	
	if (isListeningSocket(currEvent.data.fd)) {
		manageListenSocketEvent(currEvent);
	}
	else if (currEvent.events & EPOLLIN) {
		readRequest(currEvent.data.fd);
	}
	else if (currEvent.events & EPOLLOUT) {
		sendResponse(currEvent.data.fd);
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
		
		prepResponse(clientFd);
		
		std::cout <<_fdClientDataMap[clientFd].clientResponse<< "\n";
		setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
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
	const char*	servResp = _fdClientDataMap[clientFd].clientResponse.c_str();
	size_t		responseSize = strlen(servResp);
	ssize_t bytesSent = send(clientFd, servResp + totalBytesSent, responseSize - totalBytesSent, 0);

	if (bytesSent == -1) {
		setEpollCtl(clientFd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}
	totalBytesSent += bytesSent;
	if (totalBytesSent == strlen(servResp)) {
		std::cout << "All data sent: Connection closed" << "\n";
		setEpollCtl(clientFd, EPOLLIN, EPOLL_CTL_MOD);
		totalBytesSent = 0;

		close(clientFd);
		_fdClientDataMap.erase(clientFd);
	}
}




void	ServerManager::prepResponse(int clientFd) {

	_fdClientDataMap[clientFd].clientResponse = "Hell world!";
}