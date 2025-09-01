/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 21:46:15 by amysiv            #+#    #+#             */
/*   Updated: 2025/08/24 22:29:16 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#define EPOLL_CAPACITY            20
#define DEFAULT_CONFIG_FILE_PATH  "./webserv.conf"
#define ENABLE                    1
#define DISABLE                   0
#define NONE                      0

#include "parsingConfFile/ParseConfig.hpp"
#include "parsingConfFile/vServer.hpp"
#include "Request/Request.hpp"
#include "CGIHandler/CGIHandler.hpp"

#include <fstream>
#include <map>
#include <unordered_map>
#include <cerrno>
#include <cstring>

// system types
#include <netdb.h>        // addrinfo
#include <sys/epoll.h>    // epoll_event

// forward decls to avoid circular includes
class Client;
class Server;
struct addrinfo;
struct epoll_event;

/**
 * @brief Global flag toggled by signal handlers to stop the main loop.
 */
extern volatile sig_atomic_t running;

/**
 * @brief Manages configuration, servers, epoll loop, and client lifecycle.
 */
class ServerManager {
  private:
    std::ifstream                                       _configFileFd;
    int                                                 _epollFd;
    std::vector<vServer>                                _vServers;
    std::map<std::string, std::vector<const vServer*>>  _hostVserverMap;
    std::vector<Server>                                 _servers;
    std::map<int, Client>                               _fdClientMap;
    std::unordered_map<int, Client*>                    _cgiFdClientPtrMap;

    /**
     * @brief Get config file stream.
     * @return Reference to config file stream.
     */
    std::ifstream& getConfigFileFd(void);

    /**
     * @brief Get epoll file descriptor.
     * @return Epoll fd.
     */
    int getEpollFd(void) const;

    /**
     * @brief Get socket fd for host/port.
     * @param host Hostname or IP.
     * @param port Port string.
     * @return Socket fd.
     */
    int getSocketFd(const std::string& host, const std::string& port);

    /**
     * @brief Get address list for host/port.
     * @param host Hostname or IP.
     * @param port Port string.
     * @return addrinfo list pointer.
     */
    addrinfo* getAddrList(const std::string& host, const std::string& port);

	
    /**
	 * @brief Add sockets to epoll read set.
     */
	void setSocketsToEpollIn(void);
	
    /**
	 * @brief Control epoll operations.
	 * @param targetFd File descriptor.
	 * @param eventFlag Epoll events.
	 * @param operation Epoll control operation.
     */
	void setEpollCtl(int targetFd, int eventFlag, int operation);
	
    /**
	 * @brief Set fd to non-blocking mode.
	 * @param fd File descriptor.
     */
	static void setNonBlocking(int fd);
	
    /**
	 * @brief Bind socket to address list.
     * @param addrList Address list.
	 * @return Bound socket fd.
     */
	int bindSocket(addrinfo* addrList);
	
    /**
	 * @brief Handle event on listening socket.
	 * @param epollEvents Epoll event struct.
     */
	void manageListenSocketEvent(const struct epoll_event& epollEvents);
	
    /**
	 * @brief Handle generic epoll event.
	 * @param epollEvents Epoll event struct.
     */
	void manageEpollEvent(const struct epoll_event& epollEvents);
	
    /**
	 * @brief Add client to map.
	 * @param clientFd Client fd.
	 * @param serverFd Server fd.
     */
	void addClientToMap(int clientFd, int serverFd);
	
    /**
	 * @brief Check if fd is listening socket.
	 * @param fd File descriptor.
	 * @return True if listening socket.
     */
	bool isListeningSocket(int fd);
	
    /**
	 * @brief Check if fd is client socket.
	 * @param fd File descriptor.
	 * @return True if client socket.
     */
	bool isClientSocket(int fd);
	
    /**
	 * @brief Close client connection.
     * @param clientFd Client fd.
     */
	void closeClientFd(int clientFd);
	
	public:
    /**
	 * @brief Construct manager from config file.
	 * @param ConfigFileName Path to config file.
	 * @param epollSize Initial epoll size.
     */
	ServerManager(char* ConfigFileName, int epollSize);
	
    /** @brief Destructor. */
    ~ServerManager();



	/**
	 * @brief Initialize servers from configuration.
	 */
	void setServers();




	
    /**
	 * @brief Start main server loop.
     */
	void runServers(void);
	
    /**
     * @brief Get client map.
     * @return Map of fd to Client.
     */
    std::map<int, Client>& getFdClientMap(void);

    /**
     * @brief Get all servers.
     * @return Vector of servers.
     */
    std::vector<Server>& getServers(void);

    /**
     * @brief Get virtual servers (parsed config).
     * @return Reference to vector of virtual servers.
     */
    std::vector<vServer>& getVirtualServers(void);

    /**
     * @brief Parse configuration file.
     * @param _vServers Vector to store parsed servers.
     */
    void parsConfigFile(std::vector<vServer>& _vServers);

    /**
     * @brief Group servers by host.
     * @param _vServers Vector of virtual servers.
     */
    void groupServers(const std::vector<vServer>& _vServers);

    /**
     * @brief Close all sockets.
     */
    void closeAllSockets();

    /**
     * @brief Map CGI fd to client.
     * @param cgiFd CGI process fd.
     * @param clientFd Client fd.
     */
    void addCgiFdToMap(int cgiFd, int clientFd);

    /**
     * @brief Find server configs by fd.
     * @param serverFd Server fd.
     * @return Vector of vServers for that fd.
     */
    const std::vector<const vServer*> findServerConfigsByFd(int serverFd) const;

    /**
     * @brief Find server config by name.
     * @param subConfigs Sub server configs.
     * @param serverName Server name.
     * @return Matching vServer or nullptr.
     */
    const vServer* findServerConfigByName(const std::vector<const vServer*>& subConfigs, std::string serverName) const;

    /**
     * @brief Find location block by URI.
     * @param serverConfig Server configuration.
     * @param url Request URI.
     * @return Matching Location or nullptr.
     */
    const Location* findLocationBlockByUri(const vServer& serverConfig, const std::string& url) const;

    /**
     * @brief Find default location block.
     * @param locations Map of locations.
     * @return Default Location or nullptr.
     */
    const Location* findDefaultLocationBlock(const std::map<std::string, Location>& locations) const;

    /**
     * @brief Exception for server manager errors.
     */
    class ServerManagerException : public std::exception {
      private:
        std::string _message;
      public:
        /**
         * @brief Construct exception with message.
         * @param message Error description.
         */
        ServerManagerException(const std::string& message);

        /**
         * @brief Get error message.
         * @return C-string error message.
         */
        const char* what() const noexcept override;
    };

    // Allow Client internals to access private control ops without making them public.
    friend class Client;
};

#endif