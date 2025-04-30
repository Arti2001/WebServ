/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 10:03:46 by pminialg          #+#    #+#             */
/*   Updated: 2025/04/30 11:29:43 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "core/Server.hpp"


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

void	parsConfigFile(char *file) {

	ParseConfig					parser(file);
	std::vector<std::string>	roughData;

	
	try{
		parser.openConfigFile();
		roughData = parser.prepToTokenizeConfigData();
		parser.tokenizeConfigData(roughData);
		parser.parsConfigFileTokens();
	}catch(ParseConfig::ConfException& ex){
		std::cerr << "Error: " << ex.what()<< "\n";
		return ;
	}
	
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return (0);
		
	parsConfigFile(argv[1]);
	

		

	
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
