/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 10:03:46 by pminialg          #+#    #+#             */
/*   Updated: 2025/03/30 12:47:38 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "core/Server.hpp"
#include <iostream>
#include <csignal>

Server *g_server = nullptr;
bool g_running = true;


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
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	// Create and initialise server
	Server server("8080", "127.0.0.1");
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
