#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {
	// Default constructor
	registerDirectives(); // Register directives
}

ServerConfig::ServerConfig(const ServerConfig &src) : _listen(src._listen), _server_name(src._server_name), _error_page(src._error_page), _client_max_body_size(src._client_max_body_size), autoindex(src.autoindex), _locations(src._locations), _index(src._index), _root(src._root) {
	// Copy constructor
}

ServerConfig &ServerConfig::operator=(const ServerConfig &src) {
	// Assignment operator
	if (this != &src) {
		_listen = src._listen;
		_server_name = src._server_name;
		_error_page = src._error_page;
		_client_max_body_size = src._client_max_body_size;
		autoindex = src.autoindex;
		_locations = src._locations;
		_index = src._index;
		_root = src._root;
	}
	return *this;
}

ServerConfig::~ServerConfig() {
	// Destructor
}

void ServerConfig::addServerName(const std::string &server_name) {
	if (std::find(_server_name.begin(), _server_name.end(), server_name) == _server_name.end()) {
		_server_name.push_back(server_name);
	}
}
void ServerConfig::addErrorPage(int error_code, const std::string &error_page) {
	_error_page[error_code] = error_page;
}

void ServerConfig::addIndex(const std::string &index) {
	_index.push_back(index);
}


void ServerConfig::setClientMaxBodySize(size_t client_max_body_size) {
	_client_max_body_size = client_max_body_size;
}
void ServerConfig::setAutoindex(bool autoindex) {
	this->autoindex = autoindex;
}
void ServerConfig::addLocation(const std::string &path, const LocationConfig &location) {
	if (_locations.find(path) != _locations.end()) {
		std::cerr << "Error: Location " << path << " already exists." << std::endl;
		exit(1); // has to be an error. need to log in properly
	}
	_locations[path] = location;
}

const std::vector<std::string> &ServerConfig::getServerName() const {
	return _server_name;
}
const std::unordered_map<int, std::string> &ServerConfig::getErrorPage() const {
	return _error_page;
}
const uint64_t &ServerConfig::getClientMaxBodySize() const {
	return _client_max_body_size;
}
const std::unordered_map<std:: string, LocationConfig> &ServerConfig::getLocations() const {
	return _locations;
}

bool ServerConfig::getAutoindex() const {
	return autoindex;
}

const std::unordered_map<std::string, ListenConfig> &ServerConfig::getListen() const {
	return _listen;
}
const std::vector<std::string> &ServerConfig::getIndex() const {
	return _index;
}

const std::string &ServerConfig::getRoot() const {
	return _root;
}
void ServerConfig::setRoot(const std::string &root) {
	_root = root;
}

void ServerConfig::addListen(std::string &listen) {
	if (_listen.find(listen) != _listen.end()) {
		std::cerr << "Error: Listen line " << listen << " already exists." << std::endl;
		exit(1); // Listen line already exists
	}
	_listen[listen] = ListenConfig(listen);
}

void ServerConfig::registerDirectives() {
	_directiveHandlers["listen"] = [this](const std::vector<std::string>& tokens) {
		return handleListen(tokens);
	};
	_directiveHandlers["server_name"] = [this](const std::vector<std::string>& tokens) {
		return handleServerName(tokens);
	};
	_directiveHandlers["error_page"] = [this](const std::vector<std::string>& tokens) {
		return handleErrorPage(tokens);
	};
	_directiveHandlers["client_max_body_size"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("client_max_body_size")) {
			std::cerr << ("Duplicate 'client_max_body_size' directive found") << std::endl;
			exit(1);
		}
		_seenDirectives.insert("client_max_body_size");
		return handleClientMaxBodySize(tokens);
	};
	_directiveHandlers["autoindex"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("autoindex")) {
			std::cerr << "Duplicate 'autoindex' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("autoindex");
		return handleAutoindex(tokens);
	};

	_directiveHandlers["index"] = [this](const std::vector<std::string>& tokens) {
		return handleIndex(tokens);
	};
	_directiveHandlers["root"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("root")) {
			std::cerr << "Duplicate 'root' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("root");
		return handleRoot(tokens);
	};
}

bool ServerConfig::handleDirective(const std::string& directive, const std::vector<std::string>& tokens) {
	auto it = _directiveHandlers.find(directive);
	if (it != _directiveHandlers.end()) {
		return it->second(tokens);
	}
	return false; // Directive not found
}

bool ServerConfig::handleListen(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		std::cerr << "Error: Invalid number of arguments for listen directive." << tokens.size() << std::endl;
		return false; // Invalid number of arguments
	}
	std::string listenLine = tokens[0];
	ListenConfig config(listenLine);
	if (_listen.find(listenLine) != _listen.end()) {
		std::cerr << "Error: Listen line " << listenLine << " already exists." << std::endl;
		return false; // Listen line already exists
	}
	_listen[config.getListenLine()] = config;        // normalized key used

	return true;
}

bool ServerConfig::handleIndex(const std::vector<std::string>& tokens) {
	if (tokens.size() < 1) {
		return false; // Invalid number of arguments
	}
	// if it is a duplicate, we need to override the old one
	_index.clear();
	for (size_t i = 0; i < tokens.size(); ++i) {
		addIndex(tokens[i]);
	}
	return true;
}

bool ServerConfig::handleServerName(const std::vector<std::string>& tokens) {
	if (tokens.size() < 1) {
		return false; // Invalid number of arguments. We don;t seem to care about duplicates
	}
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (std::find(_server_name.begin(), _server_name.end(), tokens[i]) != _server_name.end()) {
			std::cerr << "Error: Server name " << tokens[i] << " already exists." << std::endl;
			return false; // Duplicate server name
		}
		addServerName(tokens[i]);
	}
	return true;
}

bool ServerConfig::handleErrorPage(const std::vector<std::string>& tokens) {
	if (tokens.size() != 2) {
		return false; // Invalid number of arguments
	}
	int error_code = std::stoi(tokens[0]);
	if (error_code < 400 || error_code > 599) {
		std::cerr << "Error: Invalid error code " << error_code << "." << std::endl;
		return false; // Invalid error code
	}
	addErrorPage(error_code, tokens[1]);
	return true;
}

bool ServerConfig::handleClientMaxBodySize(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	std::string suffix = "";
	
	for (size_t i = 0; i < tokens[0].size(); ++i) {
		if (!isdigit(tokens[0][i])) 
		{
			if ((tokens[0][i] == 'k' || tokens[0][i] == 'K' || tokens[0][i] == 'm' || tokens[0][i] == 'M' || tokens[0][i] == 'g' || tokens[0][i] == 'G') && suffix.empty()) {
				suffix = tokens[0].substr(i);
				break;
			}
			else {
				std::cerr << "Error: Invalid size format." << std::endl;
				return false; // Invalid size format
			}
		}
	}
	int mult;
	if (suffix == "k" || suffix == "K") {
		mult = 1024;
	} else if (suffix == "m" || suffix == "M") {
		mult = 1024 * 1024;
	} else if (suffix == "g" || suffix == "G") {
		mult = 1024 * 1024 * 1024;
	} else {
		mult = 1;
	}
	uint64_t final_size = std::stoull(tokens[0]) * mult;
	if (final_size < std::stoull(tokens[0])) {
		std::cerr << "Error: Size overflow." << std::endl;
		return false; // Size exceeds maximum limit
	}
	if (final_size == 0) {
		setClientMaxBodySize(ULLONG_MAX); // Set to max value if 0 is provided
		return true;
	}
	setClientMaxBodySize(final_size); // need to add handling of suffixes like k, m, g
	return true;
}

bool ServerConfig::handleAutoindex(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	if (tokens[0] == "on") {
		setAutoindex(true);
	} else if (tokens[0] == "off") {
		setAutoindex(false);
	} else {
		return false; // Invalid value for autoindex
	}
	return true;
}

bool ServerConfig::handleRoot(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	setRoot(tokens[0]);
	return true;
}

void ServerConfig::setDefaults() {
	if (_server_name.empty()) {
		_server_name.push_back("default_server");
	}
	if (_client_max_body_size == 0) {
		_client_max_body_size = 1048576; // Default to 1MB
	}
	if (_root.empty()) {
		_root = "./"; // Default root directory
	}
	if (_index.empty()) {
		_index.push_back("index.html"); // Default index file
	}
	if (autoindex == false) {
		autoindex = false; // Default autoindex setting
	}
	if (_listen.empty()) {
		std::string defaultListen = "80";
		_listen[defaultListen] = ListenConfig(defaultListen); // Default listen configuration
	}
	if (_locations.empty()) {
		_locations["/"] = LocationConfig(*this);
	}
	
}