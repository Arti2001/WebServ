/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 10:55:44 by amysiv            #+#    #+#             */
/*   Updated: 2025/09/17 10:55:47 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */




#ifndef SERVER_HPP
#define SERVER_HPP




#define QUEUE_LENGTH	10   ///< Maximum length of the pending connections queue
#define IN				1    ///< Direction flag: incoming
#define OUT				2    ///< Direction flag: outgoing




#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <csignal>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <cerrno>
#include <bits/stdc++.h>
#include "parsingConfFile/ParseConfig.hpp"




/**
 * @class Server
 * @brief Represents a server socket and its associated virtual server configurations.
 */
class Server
{
	private:
		int _socketFd;   ///< File descriptor of the server socket
		std::vector<const vServer*> _servConfigs; ///< Configurations for the virtual servers hosted on this socket

	public:
		/**
		 * @brief Construct a new Server object
		 * @param socketFd File descriptor of the server socket
		 * @param vServers Vector of virtual server configurations
		 */
		Server(int socketFd, std::vector<const vServer*>& vServers);

		/**
		 * @brief Destroy the Server object, closing resources if necessary
		 */
		~Server();

		/**
		 * @brief Get the socket file descriptor
		 * @return int The server socket's file descriptor
		 */
		int getSocketFd(void) const;

		/**
		 * @brief Get the vector of virtual server configurations
		 * @return const std::vector<const vServer*>& Reference to server configuration vector
		 */
		const std::vector<const vServer*>& getServConfigs(void) const;
};

#endif
