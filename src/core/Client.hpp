#ifndef CLIENT_HPP
#define CLIENT_HPP
#define REQUEST_READ_BUFFER			8192


#include "Request/Request.hpp"
#include "ServerManager.hpp"
#include "Server.hpp"
#include "Response/Response.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

class ServerManager;
class RequestParser;

class Client{

	private:
		//
		Request					_request;
		std::string				_startLineAndHeadersBuffer;
		std::string				_bodyBuffer;
		bool					_headersParsed;
		size_t 					_bodyStart;

		size_t					_clientBytesSent;
		Response				_response;
		int						_serverFd;
		ServerManager*			_serverManager;
		std::time_t				_lastActiveTime;
		bool					_closed;

		bool					headersComplete(const std::string& request);
		bool 					bodyComplete(const std::string& body) const;
	public:
		Client(int	serverFd, ServerManager* servManager);
		~Client();

		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;

		//setter
		void	setLastActiveTime(std::time_t timeStamp);
		void	setIsClosed(bool flag);


		//getters
		std::time_t			getLastActiveTime( void ) const;
		//bool&				getIsClosed(void);
		int					getServerFd( void ) const;
		size_t&				getClientsBytesSent( void );
		Request&			getRequest( void ) { return (_request); }
		Response&			getResponse(void) {return (_response);}

		//methods


		void					handleRequest( int clientFd );
		void					handleResponse( int clientFd );
		
		std::string				getResponse(Request &request);
		std::string				getCgiResponse(Request &request);

		std::string				getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName);

};


#endif