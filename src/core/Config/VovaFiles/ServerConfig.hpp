#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include "ListenConfig.hpp"
#include "Webserv.hpp"

class ServerConfig
{
	private:
		std::unordered_map<std::string, ListenConfig> _listen; // Listen configuration for the server
		std::vector<std::string> _server_name; // Server name
		std::unordered_map<int, std::string> _error_page; // Error page for the server (error code -> page)
		uint64_t _client_max_body_size; // Maximum body size for client requests, default if not overridden in locations
		bool autoindex; // Autoindex setting for the server, default if not overridden in locations
		std::unordered_map<std::string, LocationConfig> _locations; // Locations for the server
		std::vector<std::string> _index; // Default index file for the server
		std::string _root; // Root directory for the server

		std::map<std::string, std::function<bool(const std::vector<std::string>&)>> _directiveHandlers;
		void registerDirectives();
		bool handleListen(const std::vector<std::string>& tokens);
		bool handleServerName(const std::vector<std::string>& tokens);
		bool handleErrorPage(const std::vector<std::string>& tokens);
		bool handleClientMaxBodySize(const std::vector<std::string>& tokens);
		bool handleAutoindex(const std::vector<std::string>& tokens);
		bool handleIndex(const std::vector<std::string>& tokens);
		bool handleRoot(const std::vector<std::string>& tokens);
		std::unordered_set<std::string> _seenDirectives; // Set of seen directives to avoid duplicates


	public:
		ServerConfig();
		ServerConfig(const ServerConfig &src);
		ServerConfig &operator=(const ServerConfig &src);
		~ServerConfig();

		const std::vector<std::string> &getServerName() const;
		const std::unordered_map<int, std::string> &getErrorPage() const;
		const uint64_t &getClientMaxBodySize() const;
		const std::unordered_map<std::string, LocationConfig> &getLocations() const;
		bool getAutoindex() const;
		const std::unordered_map<std::string, ListenConfig> &getListen() const;
		const std::vector<std::string> &getIndex() const;
		const std::string &getRoot() const;
		
		void addServerName(const std::string &server_name);
		void addErrorPage(int error_code, const std::string &error_page);
		void addLocation(const std::string &path, const LocationConfig &location);
		void addIndex(const std::string &index);
		void addListen(std::string &listen);
		void setRoot(const std::string &root);

		void setClientMaxBodySize(size_t client_max_body_size);
		void setAutoindex(bool autoindex);
		
		bool handleDirective(const std::string& directive, const std::vector<std::string>& tokens);
		void setDefaults(); // Set default values for the server configuration


};

#endif
