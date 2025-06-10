#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) : _filename(filename) {
	// Constructor with filename
	addSupportedMethods();
	std::cout << "ConfigParser constructor called with filename: " << filename << std::endl;
}

ConfigParser::ConfigParser(const ConfigParser &src) : _filename(src._filename), _servers(src._servers){
	// Copy constructor
	std::cout << "ConfigParser copy constructor called" << std::endl;
}

ConfigParser &ConfigParser::operator=(const ConfigParser &src) {
	// Assignment operator
	std::cout << "ConfigParser assignment operator called" << std::endl;
	if (this != &src) {
		_filename = src._filename;
		_servers = src._servers;
	}
	return *this;
}

void ConfigParser::addSupportedMethods(void){
	_supportedMethods.push_back("GET");
	_supportedMethods.push_back("POST");
	_supportedMethods.push_back("DELETE");
}

ConfigParser::~ConfigParser() {
	// Destructor
	std::cout << "ConfigParser destructor called" << std::endl;
}

std::string ConfigParser::readBlock(std::istream& in, std::string &line)
{
	// Read a block of text from the file until the closing brace
	std::string block;
	int braceCount = 0;

	// Add current line (which includes "server {")
	block += line + "\n";

	// Check if line contains opening brace
	if (line.find('{') != std::string::npos) {
		braceCount++;
	}
	
	// Read lines until we close all opened braces
	while (braceCount > 0 && std::getline(in, line)) {
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}

		block += line + "\n";

		// Count opening/closing braces
		braceCount += std::count(line.begin(), line.end(), '{');
		braceCount -= std::count(line.begin(), line.end(), '}');
	}
	if (braceCount != 0) {
		std::cerr << "Error: Mismatched braces in block." << std::endl;
		return "";
	}
	return block;
}

void ConfigParser::parse() {
	std::cout << "Parsing configuration file: " << _filename << std::endl;
	std::ifstream file(this->_filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << _filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue; // Skip empty lines and comments
		}
		if (line.find("server") == 0) { // Make sure 'server' is at the start of the line
			std::string serverBlock;
			serverBlock = readBlock(file, line);
			if (serverBlock.empty()) {
				std::cerr << "Error: Failed to read server block." << std::endl;
				continue;
			}
			ServerConfig serverConfig = parseServerBlock(serverBlock);
			_servers.push_back(serverConfig);
		}
	}
	if (_servers.empty()) {
		std::cerr << "Error: No valid server blocks found." << std::endl;
	}
	file.close();
}

std::pair<std::string, std::vector<std::string>> ConfigParser::tokenizeLine(std::string& line) {
	// Remove comments
	size_t commentPos = line.find('#');
	if (commentPos != std::string::npos) {
		line = line.substr(0, commentPos);
	}
    trim(line);

	// Remove semicolon if present
	if (line.find('{') == std::string::npos && line.find('}') == std::string::npos) {
        if (line.empty() || line.back() != ';') {
            std::cerr << "Error: Missing semicolon: " << line << std::endl;
            return std::make_pair("", std::vector<std::string>());
        }
        line = line.substr(0, line.size() - 1); // remove semicolon
        trim(line);
    }

	std::istringstream ss(line);
	std::string directive;
	ss >> directive;

	std::vector<std::string> args;
	std::string arg;
	while (ss >> arg) {
		args.push_back(arg);
	}

	return std::make_pair(directive, args);
}

ServerConfig ConfigParser::parseServerBlock(const std::string& block) {
	// Parse a server block
	std::cout << "Parsing server block." << std::endl;
	ServerConfig serverConfig;
	std::istringstream ss(block);
	std::string line;
	int openingLine = 0;
	
	while (std::getline(ss, line)) {
		trim(line);
		if (line.empty() || line[0] == '#' || line[0] == ';') {
			continue; // Skip empty lines and comments
		}
        auto [directive, tokens] = tokenizeLine(line);
		if (directive == "server" && openingLine == 0 && tokens[0] == "{") {
			openingLine++;
			continue; // Skip the opening line
		}
		if (directive == "}") {
			break; // End of server block
		}
		if (!serverConfig.handleDirective(directive, tokens) && directive != "location") {
			std::cerr << "Server Error: Unknown directive: " << directive << std::endl;
			exit(2); // should return error as per nginx
		} else if (directive == "location") {
			std::string locationBlock = readBlock(ss, line);
			if (locationBlock.empty()) {
				std::cerr << "Error: Failed to read location block." << std::endl;
				continue;
			}
			LocationConfig locationConfig = parseLocationBlock(locationBlock, serverConfig, _supportedMethods);
			serverConfig.addLocation(locationConfig.getPath(), locationConfig);
		}		
	}
	serverConfig.setDefaults(); // Set default values for missing directives
	return serverConfig;
}

LocationConfig ConfigParser::parseLocationBlock(const std::string& block, const ServerConfig& serverConfig, const std::vector<std::string>& supportedMethods) {
	// Parse a location block
	std::cout << "Parsing location block." << std::endl;
	LocationConfig locationConfig(serverConfig);
	std::istringstream ss(block);
	std::string line;
	int openingLine = 0;

	while (std::getline(ss, line)) {
		trim(line);
		if (line.empty() || line[0] == '#') {
			continue; // Skip empty lines and comments
		}
		
		auto [directive, tokens] = tokenizeLine(line);
		if (directive == "location" && openingLine == 0 && tokens[1] == "{") {
			locationConfig.setPath(tokens[0]);
			openingLine++;
			continue; // Skip the opening line
		}
		if (directive == "}") {
			break; // End of location block
		}
		if (!locationConfig.handleDirective(directive, tokens) && directive != "allow_methods") {
			std::cerr << "Location Error: Unknown directive: " << directive << std::endl;
			exit(1); // should return error as per nginx
		}
		else if (directive == "allow_methods" && !locationConfig.handleAllowMethods(tokens, supportedMethods)) {
			std::cerr << "Location Error: Unknown method: " << tokens[0] << std::endl;
			exit(1); // should return error as per nginx
			}
		}
	locationConfig.setDefaults(); // Set default values for missing directives
	return locationConfig;
}

void ConfigParser::trim(std::string& line) {
	size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        line.clear();
        return;
    }

    size_t end = line.find_last_not_of(" \t\r\n");
    line = line.substr(start, end - start + 1);
}
void ConfigParser::handleError(const std::string& error) {
	std::cerr << "Error: " << error << std::endl;
}

void ConfigParser::printConfig() {
	// Print the parsed configuration
	std::cout << "Parsed Configuration:" << std::endl;
	for (const auto& server : _servers) {
		std::cout << "Listen:" << std::endl;
		for (const auto& listen : server.getListen()) {
			std::cout << "\t" << listen.first << ": " << listen.second.getHost() << ":" << listen.second.getPort() << std::endl;
		}
		std::cout << "Index: ";
		for (const auto& index : server.getIndex()) {
			std::cout << index << " ";
		}
		std::cout << std::endl;
		std::cout << "Root: " << server.getRoot() << std::endl;
        std::cout << "Server Names: ";
        for (const auto& serverName : server.getServerName()) {
            std::cout << serverName << " ";
        }
        std::cout << std::endl;
		std::cout << "Client Max Body Size: " << server.getClientMaxBodySize() << std::endl;
		std::cout << "Autoindex: " << (server.getAutoindex() ? "on" : "off") << std::endl;
		std::cout << "Error Pages:" << std::endl;
		for (const auto& errorPage : server.getErrorPage()) {
			std::cout << "\t" << errorPage.first << ": " << errorPage.second << std::endl;
		}
		std::cout << "Locations:" << std::endl;
		for (const auto& location : server.getLocations()) {
			std::cout << "\tLocation Path: " << location.second.getPath() << std::endl;
			std::cout << "\tRoot: " << location.second.getRoot() << std::endl;
			std::cout << "\tIndex: ";
			for (const auto& index : location.second.getIndex()) {
				std::cout << index << " ";
			}
			std::cout << std::endl;
			std::cout << "\tAutoindex: " << (location.second.getAutoindex() ? "on" : "off") << std::endl;
			std::cout << "\tAllowed Methods: ";
			for (const auto& method : location.second.getAllowedMethods()) {
				std::cout << method << " ";
			}
			std::cout << std::endl;
			std::cout << "\tUpload Path: " << location.second.getUploadPath() << std::endl;
			std::cout << "\tClient Max Body Size: " << location.second.getClientMaxBodySize() << std::endl;
			std::cout << "\tCGI Extensions:" << std::endl;
			for (const auto& cgi : location.second.getCgiExtension()) {
				std::cout << "\t\t" + cgi.first + ": " + cgi.second + "\n";
			}
			std::cout << "\tRedirect: " << location.second.getRedirect().first << " -> " << location.second.getRedirect().second << std::endl;
			std::cout<<std::endl;
		}
	}
}