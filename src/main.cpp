#include "core/Server.hpp"
#include "core/Request/Request.hpp"
#include "core/Request/RequestParser.hpp"
#include "core/Response/Response.hpp"
#include "core/Utils.hpp"
#include "core/ServerManager.hpp"


//Server *g_server = nullptr;
volatile sig_atomic_t	running = 1;


void signalHandler(int signum)
{
	std::cout << "\nInterupt signal (" << signum << ") received." << "\n";
	running = 0;
	std::cout << "Cleaning up all resources" << "\n";
	std::cout << "Server shutting down..." << "\n";
	exit(signum);
}


int main(int argc, char *argv[]) {
	if (argc > 2) {
		std::cerr<< "Usage ./webserv config_file(.conf)." << "\n";
		return (1);
	}
	try{
		ServerManager serverManager(argv[1], EPOLL_CAPACITY);
		signal(SIGINT, signalHandler);
		
		serverManager.parsConfigFile(serverManager.getVirtualServers());
		std::cout << "Configuration file parsed successfully." << "\n";
		serverManager.groupServers(serverManager.getVirtualServers());
		
		serverManager.setServers();
	
		serverManager.runServers();
		serverManager.closeAllSockets();
	}
	catch(ServerManager::ServerManagerException& ex) {

		std::cerr << "ServerManager::Error: " << ex.what()<< "\n";
		return (-1);
	}

	return 0;
}


