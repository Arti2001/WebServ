#include "ServerManager.hpp"


ServerManager::ServerManager(std::string& fileName) {

	_configFileFd.open(fileName);

	if (!_configFileFd) {

		throw (ServerManagerException("Failed to open " + fileName));
	}
	else{
		std::cout << "File: '" << fileName << "' is opend." << "\n";
	}
}

ServerManager::ServerManagerException::ServerManagerException(const std::string& msg) : _message(msg) {

}

ServerManager::~ServerManager() {

}


const char*	ServerManager::ServerManagerException::what() const noexcept {
	
	return (_message.c_str());
}

std::ifstream&	ServerManager::getConfigFileFd( void ) {

	return (_configFileFd);
}


std::vector<vServer>	ServerManager::parsConfigFile() {

	ParseConfig									parser;
	std::map<size_t, std::vector<std::string>>	roughData;

	try{
		roughData = parser.prepToTokenizeConfigData(getConfigFileFd());
		parser.tokenizeConfigData(roughData);
		parser.parsConfigFileTokens();
	}catch(ParseConfig::ConfException& ex){
		std::cerr << "Error: " << ex.what()<< "\n";
	}
	return (parser.getVSevers());
}