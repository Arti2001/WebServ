/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 10:03:46 by pminialg          #+#    #+#             */
/*   Updated: 2025/03/27 14:50:33 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./core/Server.hpp"
#include <csignal>

Server *g_server = nullptr; //pointer to our server instance, so the signal handler can access it
bool g_running = true; //a flag to control the server's main loop

/*
    Our signal handler function that gets called when the server receives a certain system signal
    
    signum: the signal number
    prints amessage about the signal
    sets g_running to false to stop the server's main loop
    calls the server's stop method if it exists
    prints a shutdown message
    exits with the signal number as the status code
*/
void signalHandler(int signum)
{
	std::cout << "\nInterupt signal (" << signum << ") received." << std::endl;
	g_running = false;
	if (g_server) {
		g_server->stop();
	}
	std::cout << "Server shutting down..." << std::endl;
	exit(signum);
}

int main(void)
{
	// setup signal handling
	signal(SIGINT, signalHandler); // handle Ctrl + C
	signal(SIGTERM, signalHandler); // handle termination request
	signal(SIGQUIT, signalHandler); // handle Ctrl +'\'

	// Create and initialise server
	Server server(8080); // creates a server that will listen on port 8080
	g_server = &server; // stores the servers address in our global pointer

	if (!server.init()) // calls init which sets up the socket, binds it, and starts listening
	{
		std::cerr << "Failed to initialize server" << std::endl;
		return 1;
	}

	std::cout << "Server running on port 8080. Press Ctrl+C to exit." << std::endl;

	// Start the server
	server.start(); // starts the server's main loop which will accept incoming connections, handle each connection, continue until interrupted

	return 0;
}

// int main(int argc, char *argv[])
// {
//     std::string config_path = "config/default.conf";

//     // Use provided config file if specified
//     if (argc > 1)
//     {
//         config_path = argv[1];
//     }

//     try
//     {
//         // TODO: Initialize server with config
//         // TODO: Start event loop
//         std::cout << "Starting webserv on port 8080..." << std::endl;

//         // TODO: Implement main server loop
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }