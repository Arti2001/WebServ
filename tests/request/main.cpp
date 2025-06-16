#include "../../src/core/Request/Request.hpp"
#include "../../src/core/Client.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " fd" << std::endl;
        return 1;
    }
    Client client(atoi(argv[1]), nullptr); // Create a client with the file descriptor
    client.handleRequest(atoi(argv[1])); // Handle the request for the client
    Request request; // Create a Request object
    request = client.getRequest(); // Get the request from the client
    if (request.checkError()) {
        std::cerr << "Error parsing request. Error code: " << request.getStatusCode() << std::endl;
        return 1; // Exit if there is an error in the request
    }
    request.printRequest();
    return 0;
}