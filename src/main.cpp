#include "core/Server.hpp"
#include "core/Request/Request.hpp"
#include "core/Request/RequestParser.hpp"
#include "core/Response/Response.hpp"
#include "core/Response/MimeTypes.hpp"
#include "core/Utils.hpp"
#include "core/ServerManager.hpp"


//Server *g_server = nullptr;
volatile sig_atomic_t	running = 1;


void signalHandler(int signum)
{
	std::cout << "\nInterupt signal (" << signum << ") received." << "\n";
	running = 0;
	std::cout << "Cleaning up all resources" << "\n";
	std::cout << "Server shutting down..." << "\n";
	exit(signum);
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr<< "Usage ./webserv config_file(.conf)." << "\n";
		return (1);
	}
	try{
		std::string fileName (argv[1]);
		ServerManager serverManager(fileName, EPOLL_CAPACITY);
		signal(SIGINT, signalHandler);
		serverManager.parsConfigFile(serverManager.getVirtualServers());
		serverManager.groupServers(serverManager.getVirtualServers());
		serverManager.setServers();
		serverManager.runServers();
		serverManager.closeAllSockets();
		
	}
	catch(ServerManager::ServerManagerException& ex) {

		std::cerr << "ServerManager::Error: " << ex.what()<< "\n";
		return (1);
	}


	
	
	

	

	
	
	//signal(SIGINT, signalHandler);
	//signal(SIGTERM, signalHandler);
	//signal(SIGQUIT, signalHandler);



	//if (!server.init()) // calls init which sets up the socket, binds it, and starts listening
	//{
	//	std::cerr << "Failed to initialize server" << "\n";
	//	return 1;
	//}

	//std::cout << "Server running on port 8080. Press Ctrl+C to exit." << "\n";

	//// Start the server
	//server.start(); // starts the server's main loop which will accept incoming connections, handle each connection, continue until interrupted
	
	return 0;
}

//int main() {
//	Response resp;
//	resp.setStatusCode(200);
//	resp.setReasonPhrase("OK");

//	resp.addHeader("Content-Type", "text/plain");
//	resp.addHeader("Connection", "close");

//	std::string msg = "Hello from Webserv!\n";
//	std::vector<char> body(msg.begin(), msg.end());
//	resp.setBody(std::move(body));

//	resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));

//	std::vector<char> out = resp.serialize();

//	std::cout.write(out.data(), out.size());
//	return 0;
//}

// int main() {
//     // 1) Build a fake request
//     // (no headers/body needed for GET)
	
//     // 2) Hard-code a Location
//     Location loc;
//     loc._path            = "/";                   // top-level route
//     loc._root            = "./www/";               // your test folder
//     loc._index           = "index.html";          // default file
//     loc._auto_index      = true;                 // no directory listing
//     loc._clientMaxSize   = 10 * 1024 * 1024;      // 10 MB
//     loc._allowedMethods  = { "GET" };     // only GET/HEAD
//     loc._errorPages      = {
// 		{404, "/errors/404.html"},
//         {403, "/errors/403.html"}
//     };
	
//     // 3) Serve it
//     StaticHandler handler;
// 	{
		
// 		HTTPRequest req;
// 		req.setMethod("GET");
// 		req.setUri   ("/files/");
// 		req.setVersion("HTTP/1.1");
// 		Response resp = handler.serve(req, loc);
// 		auto out = resp.serialize();
// 		std::cout << "LISTING" << std::endl;
// 		std::cout.write(out.data(), out.size());
// 		std::cout << "\n\n";
// 	}
// 	{
		
// 		HTTPRequest req;
// 		req.setMethod("GET");
// 		req.setUri   ("/does-not-exist");
// 		req.setVersion("HTTP/1.1");
// 		Response resp = handler.serve(req, loc);
// 		auto out = resp.serialize();
// 		std::cout << "CUSTOM ERROR" << std::endl;
// 		std::cout.write(out.data(), out.size());
// 		std::cout << "\n\n";
// 	}

    
    
//     return 0;
// }

// int main(void)
// {
// 	std::string request_encoded = 
//     "GET / HTTP/1.1\r\n"
//     "Host: localhost:8080\r\n"
//     "Content-Type: application/x-www-form-urlencoded\r\n"
//     "Content-Length: 29\r\n"
//     "\r\n"
//     "username=john&password=secret\r\n";
    
// 	std::string request_json = 
//     "GET / HTTP/1.1\r\n"
//     "Host: localhost:8080\r\n"
//     "Content-Type: application/json\r\n"
//     "Content-Length: 43\r\n"
//     "\r\n"
//     "{'username': 'john', 'password': 'secret'}\r\n";

//     std::string request_multipart =
//     "POST /upload HTTP/1.1\r\n"
//     "Host: example.com\r\n"
//     "Content-Type: multipart/form-data; boundary=34b21\r\n"
//     "Content-Length: 426\r\n"
//     "\r\n"
//     "--34b21\r\n"
//     "Content-Disposition: form-data; name='text'\r\n"
//     "Content-Type: text/plain\r\n"
//     "\r\n"
//     "Book\r\n"
//     "--34b21\r\n"
//     "Content-Disposition: form-data; name='file1'; filename='a.json'\r\n"
//     "Content-Type: application/json\r\n"
//     "\r\n"
//     "{\r\n"
//     "'title': 'Java 8 in Action',\r\n"
//     "'author': 'Mario Fusco',\r\n"
//     "'year': 2014\r\n"
//     "}\r\n"
//     "--34b21\r\n"
//     "Content-Disposition: form-data; name='file2'; filename='a.html'\r\n"
//     "Content-Type: text/html\r\n"
//     "\r\n"
//     "<title> Available for download! </title>\r\n"
//     "--34b21--\r\n";
    
    
//     // Print the request as is
//     // std::cout << request;
//     std::unordered_map<int, HTTPRequest> resultMap;
    
//     RequestParser parser;
//     // HTTPRequest rrr;
//     parser.handleIncomingRequest(1, request_encoded, resultMap);
//     parser.handleIncomingRequest(2, request_json, resultMap);
//     parser.handleIncomingRequest(3, request_multipart, resultMap);
// // parser.handleIncomingRequest(1, request2, resultMap);
// // resultMap = parser.handleIncomingRequest(1, request, resultMap);
//     std::cout << resultMap[1] << std::endl;
//     std::cout << "----------" << std::endl;
//     std::cout << resultMap[2] << std::endl;
//     std::cout << "----------" << std::endl;
//     std::cout << resultMap[3] << std::endl;
// }


// #include <iostream>
// #include <string>
// #include <unordered_map>
// #include "core/Request/RequestParser.hpp"
// #include "core/Request/Request.hpp"

// // Helper function to create a chunked HTTP request
// std::string createChunkedRequest(const std::string& body) {
//     std::string headers = 
//         "POST /test HTTP/1.1\r\n"
//         "Host: localhost:8080\r\n"
//         "Transfer-Encoding: chunked\r\n"
//         "Content-Type: text/plain\r\n"
//         "\r\n";
    
//     std::string chunked_body;
    
//     // Break the body into chunks (here we use 5-byte chunks for demonstration)
//     size_t pos = 0;
//     while (pos < body.length()) {
//         size_t chunk_size = std::min(5UL, body.length() - pos);
//         std::stringstream chunk_header;
//         chunk_header << std::hex << chunk_size << "\r\n";
        
//         chunked_body += chunk_header.str();
//         chunked_body += body.substr(pos, chunk_size) + "\r\n";
//         pos += chunk_size;
//     }
    
//     // Add final zero-size chunk
//     chunked_body += "0\r\n\r\n";
    
//     return headers + chunked_body;
// }

// // Prints out the details of an HTTP request
// void printRequest(const HTTPRequest& req) {
//     std::cout << "Method: " << req.getMethod() << std::endl;
//     std::cout << "URI: " << req.getUri() << std::endl;
//     std::cout << "Headers:" << std::endl;
//     for (const auto& header : req.getHeaders()) {
//         std::cout << "  " << header.first << ": " << header.second << std::endl;
//     }
//     std::cout << "Body size: " << req.getBody().size() << " bytes" << std::endl;
//     std::cout << "Body content: " << req.getBody() << std::endl;
// }

// int main() {
//     // Create test data
//     std::string test_body = "This is a test message that will be sent using chunked transfer encoding.";
//     std::string full_request = createChunkedRequest(test_body);
    
//     std::cout << "========= CHUNKED REQUEST DATA =========\n";
//     std::cout << full_request << std::endl;
//     std::cout << "========================================\n\n";
    
//     // Simulate sending the request in multiple parts
//     RequestParser parser;
//     std::unordered_map<int, HTTPRequest> resultMap;
    
//     // Test 1: Send in one shot
//     {
//         std::cout << "TEST 1: Sending full request at once\n";
//         std::unordered_map<int, HTTPRequest> localMap;
//         parser.handleIncomingRequest(1, full_request, localMap);
        
//         if (localMap.find(1) != localMap.end()) {
//             std::cout << "✓ Request successfully parsed!\n";
//             printRequest(localMap[1]);
//         } else {
//             std::cout << "✗ Request parsing failed!\n";
//         }
//         std::cout << "\n";
//     }
    
//     // Test 2: Send in multiple parts
//     {
//         std::cout << "TEST 2: Sending request in multiple parts\n";
//         std::unordered_map<int, HTTPRequest> localMap;
        
//         // Split the request into 3 parts
//         size_t third = full_request.length() / 3;
//         std::string part1 = full_request.substr(0, third);
//         std::string part2 = full_request.substr(third, third);
//         std::string part3 = full_request.substr(2 * third);
        
//         std::cout << "Sending part 1 (" << part1.length() << " bytes)\n";
//         parser.handleIncomingRequest(2, part1, localMap);
        
//         if (localMap.find(2) != localMap.end()) {
//             std::cout << "  Request already complete after part 1 (unexpected)\n";
//         } else {
//             std::cout << "  Request incomplete after part 1 (expected)\n";
//         }
        
//         std::cout << "Sending part 2 (" << part2.length() << " bytes)\n";
//         parser.handleIncomingRequest(2, part2, localMap);
        
//         if (localMap.find(2) != localMap.end()) {
//             std::cout << "  Request already complete after part 2 (unexpected)\n";
//         } else {
//             std::cout << "  Request incomplete after part 2 (expected)\n";
//         }
        
//         std::cout << "Sending part 3 (" << part3.length() << " bytes)\n";
//         parser.handleIncomingRequest(2, part3, localMap);
        
//         if (localMap.find(2) != localMap.end()) {
//             std::cout << "✓ Request successfully parsed after all parts!\n";
//             printRequest(localMap[2]);
//         } else {
//             std::cout << "✗ Request parsing failed after all parts!\n";
//         }
//     }
    
//     return 0;
// }