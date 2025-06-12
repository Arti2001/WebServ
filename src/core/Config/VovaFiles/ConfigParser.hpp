#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserv.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

enum Method
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};

class ConfigParser {
	private:
		std::string _filename; // Name of the configuration file
		std::vector<ServerConfig> _servers; // Vector of server configurations
		std::vector<std::string> _supportedMethods; // Supported HTTP methods
		void addSupportedMethods(void); // Add supported HTTP methods

	public:
		ConfigParser(); // Default constructor
		ConfigParser(const std::string& filename); // Constructor with filename
		ConfigParser(const ConfigParser &src); // Copy constructor
		ConfigParser &operator=(const ConfigParser &src); // Assignment operator
		~ConfigParser();
		
		void parse(); // Parse the entire configuration file
		ServerConfig parseServerBlock(const std::string& block); // Parse a server block
		LocationConfig parseLocationBlock(const std::string& block, const ServerConfig &serverConfig, const std::vector<std::string> &supportedMethods); // Parse a location block
		static void trim(std::string& str); // Trim whitespace from a string
		void handleError(const std::string& error); // Handle parsing errors
		void printConfig(); // Print the parsed configuration
		std::string readBlock(std::istream& file, std::string &line); // Read a block of text from the file until the closing brace
		std::pair<std::string, std::vector<std::string>> tokenizeLine(std::string& line); // Tokenize a line into directive and arguments
		std::vector<ServerConfig> getServers() const { return _servers; } // Get the vector of server configurations
};

#endif