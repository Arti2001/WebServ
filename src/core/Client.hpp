#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include "ServerManager.hpp"
#include "parsingResponse/Response.hpp"

class ServerManager;
class RequestParser;

class Client{

	private:
		RequestParser*			_requestParser;
		std::string				_requestBuffer;
		std::string				_clientResponse;
		size_t					_clientBytesSent;
		int						_serverFd;
		ServerManager*			_serverManager;
	public:
		Client(int	serverFd, ServerManager* servManager);
		~Client();

		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;

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
		void					addToRequestBuff(char* chunk, size_t bytesread);
		
		std::string				getResponse(HTTPRequest request);
		std::string				getCgiResponse(HTTPRequest request);

		std::string				getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName);

};


#endif