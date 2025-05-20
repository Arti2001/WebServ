#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include "ServerManager.hpp"

class ServerManager;
class Client{

	private:
		std::string				_clientResponse;
		size_t					_clientBytesSent;
		time_t					_lastActiveTime;
		int						_serverFd;
		ServerManager*			_serverManager;
	
		
	public:
		Client(int	serverFd, ServerManager* servManager);
		~Client();

		//setter
		void	setLastActiveTime(std::time_t timeStamp);


		//getters
		std::time_t	getLastActiveTime( void ) const;
		int			getServerFd( void ) const;
		size_t&		getClientsBytesSent( void );
		const std::string&	getClientsResponse( void ) const;

		//methods


		void					readRequest( int clientFd );
		void					sendResponse( int clientFd );
		void					prepResponse(int clientFd );
		std::string	getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName);

};


#endif