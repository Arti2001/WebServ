#ifndef VSERVER_HPP
#define VSERVER_HPP

#include "Server.hpp"

struct Location {


};

class vServer
{
	private:
		std::string				_listen;
		std::string*			_serverName;
		std::vector<Location>	_locations;
		std::string				_root;
		
	public:
		vServer(/* args */);
		~vServer();

		void	setListen(std::string listen);
		void	setRoot(std::string root);
		void	setServName(std::string* servName);
};



#endif