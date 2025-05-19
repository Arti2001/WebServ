#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

class Client{

	private:
		//std::string				clientResponse;
		size_t					_clientBytesSent;
		int						_servFdConnectedTo;
		time_t					_lastActiveTime;
	public:
		Client(int	serverFd);
		~Client();

		void	setServFdConnectedTo();
};


#endif