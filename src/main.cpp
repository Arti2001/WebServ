/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 10:03:46 by pminialg          #+#    #+#             */
/*   Updated: 2025/05/06 19:25:47 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "core/ServerManager.hpp"


//Server *g_server = nullptr;
bool g_running = true;


//void signalHandler(int signum)
//{
//	std::cout << "\nInterupt signal (" << signum << ") received." << "\n";
//	g_running = false;
//	if (g_server) {
//		g_server->stop();
//	}
//	std::cout << "Server shutting down..." << "\n";
//	exit(signum);
//}


int main(int argc, char *argv[])
{
	
	if (argc != 2) {
		std::cerr<< "Error: Configuration file expected." << "\n";
		return (0);
	}
	try{
		std::string fileName (argv[1]);
		ServerManager serverManager(fileName);
		const std::vector<vServer>&	servers = serverManager.parsConfigFile();
	}
	catch(ServerManager::ServerManagerException& ex) {

		std::cerr << "ServerManager::Error: " << ex.what()<< "\n";
	}


	
	
	
		//const std::vector<vServer>&	servers = parsConfigFile(argv[1]);
	
		//for(size_t i = 0; servers.size()  > i; i++) {
		
		//	Server	server
			
	//	}
	

	
	
	//signal(SIGINT, signalHandler);
	//signal(SIGTERM, signalHandler);
	//signal(SIGQUIT, signalHandler);

	//Server server("8080", "127.0.0.1");
	//g_server = &server; // stores the servers address in our global pointer

	//if (!server.init()) // calls init which sets up the socket, binds it, and starts listening
	//{
	//	std::cerr << "Failed to initialize server" << "\n";
	//	return 1;
	//}

	//std::cout << "Server running on port 8080. Press Ctrl+C to exit." << "\n";

	//// Start the server
	//server.start(); // starts the server's main loop which will accept incoming connections, handle each connection, continue until interrupted
	
	return 0;
}
