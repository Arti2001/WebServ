#include "../../src/core/Config/VovaFiles/ConfigParser.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}
	
	ConfigParser configParser(argv[1]);
	configParser.parse();
	configParser.printConfig();
	
	return 0;
}