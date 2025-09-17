/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 13:08:25 by vshkonda          #+#    #+#             */
/*   Updated: 2025/09/17 10:58:03 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "core/ServerManager.hpp"


volatile sig_atomic_t	running = 1;


void signalHandler(int signum)
{
	std::cout << "\nInterupt signal (" << signum << ") received." << "\n";
	running = 0;
	std::cout << "Cleaning up all resources" << "\n";
	std::cout << "Server shutting down..." << "\n";
}


int main(int argc, char *argv[]) {
	if (argc > 2) {
		std::cerr<< "Usage ./webserv (optional | config_file(.conf))." << "\n";
		return (1);
	}
	try{
		ServerManager serverManager(argv[1], EPOLL_CAPACITY);
		signal(SIGINT, signalHandler);
		signal(SIGPIPE, SIG_IGN);
		std::vector<vServer>& virtualServers = serverManager.getVirtualServers();
		serverManager.parsConfigFile(virtualServers);
		serverManager.groupServers(virtualServers);
		serverManager.setServers();
		serverManager.runServers();
		serverManager.closeAllSockets();
	}
	catch(ServerManager::ServerManagerException& ex) {
		std::cerr << "ServerManager::Error: " << ex.what() << "\n";
		return (-1);
	}
	return 0;
}


