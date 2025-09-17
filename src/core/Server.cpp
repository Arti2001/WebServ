/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 13:08:00 by vshkonda          #+#    #+#             */
/*   Updated: 2025/09/17 10:58:56 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int socketFd, std::vector<const vServer*>& vServers) : _servConfigs(vServers){
	this->_socketFd = socketFd;
	std::cout << _servConfigs.at(0)->getServerRoot()<< "\n";

}



Server::~Server() {}





int	Server::getSocketFd(void) const {
	return (_socketFd);
}





const std::vector<const vServer*>& Server::getServConfigs( void ) const {
	return(_servConfigs);
}
