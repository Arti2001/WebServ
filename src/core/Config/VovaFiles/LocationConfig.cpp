#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

LocationConfig::LocationConfig()
{

}

LocationConfig::LocationConfig(const ServerConfig &src) : _root(src.getRoot()), _index(src.getIndex()), _autoindex(src.getAutoindex()), _client_max_body_size(src.getClientMaxBodySize()),  _error_pages(src.getErrorPage()) {
	// Constructor from ServerConfig
	registerDirectives(); // Register directives
}

LocationConfig::LocationConfig(const LocationConfig &src) : _path(src._path), _root(src._root), _index(src._index), _autoindex(src._autoindex), _allow_methods(src._allow_methods), _cgi_extension(src._cgi_extension), _upload_path(src._upload_path), _client_max_body_size(src._client_max_body_size), _redirect(src._redirect) {
	// Copy constructor
}

LocationConfig &LocationConfig::operator=(const LocationConfig &src) {
	// Assignment operator
	if (this != &src) {
		_path = src._path;
		_root = src._root;
		_index = src._index;
		_autoindex = src._autoindex;
		_allow_methods = src._allow_methods;
		_cgi_extension = src._cgi_extension;
		_upload_path = src._upload_path;
		_client_max_body_size = src._client_max_body_size;
		_redirect = src._redirect;
	}
	return *this;
}

LocationConfig::~LocationConfig() {
	// Destructor
}


const std::string &LocationConfig::getPath() const {
	return _path;
}

const std::string &LocationConfig::getRoot() const {
	return _root;
}

const std::vector<std::string> &LocationConfig::getIndex() const {
	return _index;
}

const std::unordered_set<std::string> &LocationConfig::getAllowedMethods() const {
	return _allow_methods;
}

const std::unordered_map<std::string, std::string> &LocationConfig::getCgiExtension() const {
	return _cgi_extension;
}

const std::string &LocationConfig::getUploadPath() const {
	return _upload_path;
}

const std::unordered_map<int, std::string> &LocationConfig::getErrorPages() const {
	return _error_pages;
}

const uint64_t &LocationConfig::getClientMaxBodySize() const {
	return _client_max_body_size;
}

void LocationConfig::setPath(const std::string &path) {
	_path = path;
}

void LocationConfig::setRoot(const std::string &root) {
	_root = root;
}

void LocationConfig::addIndex(const std::string &index) {
	_index.push_back(index);
}

void LocationConfig::addAllowedMethod(const std::string &method) {
	_allow_methods.insert(method);
}

void LocationConfig::addCgiExtension(const std::string &extension, const std::string &path) {
	_cgi_extension[extension] = path;
}

void LocationConfig::addErrorPage(int error_code, const std::string &error_page) {
	_error_pages[error_code] = error_page;
}

void LocationConfig::setUploadPath(const std::string &upload_path) {
	_upload_path = upload_path;
}

void LocationConfig::setClientMaxBodySize(uint64_t client_max_body_size) {
	_client_max_body_size = client_max_body_size;
}

void LocationConfig::setRedirect(int code, const std::string &path) {
	_redirect = std::make_pair(code, path);
}

bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

const std::pair<int, std::string> &LocationConfig::getRedirect() const {
	return _redirect;
}

void LocationConfig::setAutoindex(bool autoindex) {
	_autoindex = autoindex;
}

void LocationConfig::registerDirectives() {

	_directiveHandlers["path"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("path")) {
			std::cerr << "Duplicate 'path' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("path");
		return handlePath(tokens);
	};
	_directiveHandlers["root"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("root")) {
			std::cerr << "Duplicate 'root' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("root");
		return handleRoot(tokens);
	};
	_directiveHandlers["index"] = [this](const std::vector<std::string>& tokens) {
		return handleIndex(tokens);
	};
	_directiveHandlers["autoindex"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("autoindex")) {
			std::cerr << "Duplicate 'autoindex' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("autoindex");
		return handleAutoindex(tokens);
	};
	_directiveHandlers["cgi_extension"] = [this](const std::vector<std::string>& tokens) {
		return handleCgiExtension(tokens);
	};
	_directiveHandlers["client_max_body_size"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("client_max_body_size")) {
			std::cerr << "Duplicate 'client_max_body_size' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("client_max_body_size");
		return handleClientMaxBodySize(tokens);
	};
	_directiveHandlers["return"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("return")) {
			std::cerr << "Duplicate 'return' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("return");
		return handleRedirect(tokens);	
	};
	_directiveHandlers["upload_path"] = [this](const std::vector<std::string>& tokens) {
		if (_seenDirectives.count("upload_path")) {
			std::cerr << "Duplicate 'upload_path' directive found" << std::endl;
			exit(1);
		}
		_seenDirectives.insert("upload_path");
		return handleUploadPath(tokens);
	};
}

bool LocationConfig::handlePath(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	setPath(tokens[0]);
	return true;
}

bool LocationConfig::handleRoot(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	setRoot(tokens[0]);
	return true;
}

bool LocationConfig::handleIndex(const std::vector<std::string>& tokens) {
	if (tokens.size() < 1) {
		return false; // Invalid number of arguments
	}
	// if it is already exists, we need to overwrite it
	_index.clear();
	for (size_t i = 0; i < tokens.size(); ++i) {
		addIndex(tokens[i]);
	}
	return true;
}

bool LocationConfig::handleAutoindex(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	setAutoindex(tokens[0] == "on");
	return true;
}

bool LocationConfig::handleAllowMethods(const std::vector<std::string>& tokens, const std::vector<std::string>& supportedMethods) {
	// Check if the method is in the list of supported methods
	if (tokens.size() < 1) {
		return false; // Invalid number of arguments
	}
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (std::find(supportedMethods.begin(), supportedMethods.end(), tokens[i]) == supportedMethods.end()) {
			std::cerr << "Error: Unsupported method: " << tokens[i] << std::endl;
			exit(1); // Unsupported method
		}
		addAllowedMethod(tokens[i]);
	}
	return true;
}

bool LocationConfig::handleCgiExtension(const std::vector<std::string>& tokens) {
	if (tokens.size() != 2) {
		return false; // Invalid number of arguments
	}

	addCgiExtension(tokens[0], tokens[1]);
	return true;
}

bool LocationConfig::handleUploadPath(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	setUploadPath(tokens[0]);
	return true;
}

bool LocationConfig::handleClientMaxBodySize(const std::vector<std::string>& tokens) {
	if (tokens.size() != 1) {
		return false; // Invalid number of arguments
	}
	size_t size = std::stoul(tokens[0]); // limit is 64-bit unsigned int
	if (size == 0) {
		setClientMaxBodySize(ULLONG_MAX); // Set to max value if 0 is provided
		return true;
	}
	setClientMaxBodySize(size);
	return true;
}
 // need to make a list of methods that are available. then check if the method that is in the directive in in this list
 // 

bool LocationConfig::handleRedirect(const std::vector<std::string>& tokens) {
	if (tokens.size() != 2) {
		return false; // Invalid number of arguments
	}
	int status_code = std::stoi(tokens[0]);
	if (status_code < 300 || status_code > 399) {
		std::cerr << "Error: Invalid redirect status code " << status_code << "." << std::endl;
		return false; // Invalid redirect status code
	}
	std::string redirect_path = tokens[1];
	setRedirect(status_code, redirect_path);
	return true;
}

// check if the method is in the list of allowed methods
bool LocationConfig::handleDirective(const std::string& directive, const std::vector<std::string>& tokens) {
	auto it = _directiveHandlers.find(directive);
	if (it != _directiveHandlers.end()) {
		return it->second(tokens);
	}
	return false; // Unknown directive
}

void LocationConfig::setDefaults() {
	// Set default values for the location configuration
	if (_upload_path.empty()) {
		_upload_path = "/var/www/uploads";
	}
	if (_redirect.first == 0) {
		setRedirect(301, "/");
	}
	if (_error_pages.empty()) {
		addErrorPage(404, "/404.html");
	}
	if (_allow_methods.empty()) {
		addAllowedMethod("GET");
	}
	if (_cgi_extension.empty()) {
		addCgiExtension(CGI_EXTENSION, "/usr/bin/python3");
	}
}