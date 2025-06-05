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
		//bool					_closed;

	public:
		Client(int	serverFd, ServerManager* servManager);
		~Client();

		//setter
		//void	setLastActiveTime(std::time_t timeStamp);
		//void	setIsClosed(bool flag);


		//getters
		std::time_t			getLastActiveTime( void ) const;
		//bool&				getIsClosed(void);
		int					getServerFd( void ) const;
		size_t&				getClientsBytesSent( void );
		const std::string&	getClientsResponse( void ) const;

		//methods


		void					readRequest( int clientFd );
		void					sendResponse( int clientFd );
		const Response&			getResponse(HTTPRequest request);
		const Response&			getCgiResponse(HTTPRequest request);

		std::string				getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName);

};


#endif