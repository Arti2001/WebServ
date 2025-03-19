#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    std::string config_path = "config/default.conf";

    // Use provided config file if specified
    if (argc > 1)
    {
        config_path = argv[1];
    }

    try
    {
        // TODO: Initialize server with config
        // TODO: Start event loop
        std::cout << "Starting webserv on port 8080..." << std::endl;

        // TODO: Implement main server loop
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}