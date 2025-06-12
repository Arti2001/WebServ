#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include "Webserv.hpp"

#define CGI_EXTENSION ".py"

class ServerConfig; // Forward declaration

class LocationConfig
{
	private:
		std::string _path; // Path of the location
		std::string _root; // Root directory for the location
		std::vector<std::string> _index; // Index file for the location
		bool _autoindex; // Autoindex setting for the location
		std::unordered_set<std::string>_allow_methods; // Allowed methods for the location
		std::unordered_map<std::string, std::string> _cgi_extension; // CGI extension for the location, e.g., ".py" -> "/usr/bin/python3"
		std::string _upload_path; // Upload path for the location
		uint64_t _client_max_body_size; // Maximum body size for client requests
		std::pair<int, std::string> _redirect; // Redirect for the location, e.g., 301 -> "/new_path"
		std::unordered_set<std::string> _seenDirectives; // Set of seen directives to avoid duplicates
		std::unordered_map<int, std::string> _error_pages; // Error pages for the location (error code -> page)

		std::map<std::string, std::function<bool(const std::vector<std::string>&)>> _directiveHandlers;
		void registerDirectives();
		bool handlePath(const std::vector<std::string>& tokens);
		bool handleRoot(const std::vector<std::string>& tokens);
		bool handleIndex(const std::vector<std::string>& tokens);
		bool handleAutoindex(const std::vector<std::string>& tokens);
		bool handleCgiExtension(const std::vector<std::string>& tokens);
		bool handleUploadPath(const std::vector<std::string>& tokens);
		bool handleClientMaxBodySize(const std::vector<std::string>& tokens);
		bool handleRedirect(const std::vector<std::string>& tokens);
		bool handleErrorPage(const std::vector<std::string>& tokens);

	public:
		LocationConfig();
		LocationConfig(const ServerConfig &src);
		LocationConfig(const LocationConfig &src);
		LocationConfig &operator=(const LocationConfig &src);
		~LocationConfig();

		const std::string &getPath() const;
		const std::string &getRoot() const;
		const std::vector<std::string> &getIndex() const;
		bool getAutoindex() const;
		const std::unordered_set<std::string> &getAllowedMethods() const;
		const std::unordered_map<std::string, std::string> &getCgiExtension() const;
		const std::string &getUploadPath() const;
		const uint64_t &getClientMaxBodySize() const;
		const std::pair<int, std::string> &getRedirect() const;
		const std::unordered_map<int, std::string> &getErrorPages() const;

		void setPath(const std::string &path);
		void setRoot(const std::string &root);
		void addIndex(const std::string &index);
		void setAutoindex(bool autoindex);
		void setUploadPath(const std::string &upload_path);
		void setClientMaxBodySize(uint64_t client_max_body_size);
		void setRedirect(int code, const std::string &path);
		void addErrorPage(int error_code, const std::string &error_page);
		void addAllowedMethod(const std::string &method);
		void addCgiExtension(const std::string &extension, const std::string &path);

		bool handleAllowMethods(const std::vector<std::string>& tokens, const std::vector<std::string>& supportedMethods);
		bool handleDirective(const std::string& directive, const std::vector<std::string>& tokens);
		void setDefaults(); // Set default values for the location configuration



};

#endif