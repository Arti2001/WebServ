/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: amysiv <amysiv@student.42.fr>                +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/21 10:03:46 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/24 15:47:05 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "core/Server.hpp"
#include "core/HTTPRequest.hpp"
#include "core/RequestParser.hpp"
#include "core/Response.hpp"
#include "core/StaticHandler.hpp"
#include "core/MimeTypes.hpp"
#include "core/Utils.hpp"

Server *g_server = nullptr;
bool g_running = true;


void signalHandler(int signum)
{
	std::cout << "\nInterupt signal (" << signum << ") received." << "\n";
	g_running = false;
	if (g_server) {
		g_server->stop();
	}
	std::cout << "Server shutting down..." << "\n";
	exit(signum);
}

int main(void)
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	Server server("8080", "127.0.0.1");
	g_server = &server; // stores the servers address in our global pointer

	if (!server.init()) // calls init which sets up the socket, binds it, and starts listening
	{
		std::cerr << "Failed to initialize server" << "\n";
		return 1;
	}

	std::cout << "Server running on port 8080. Press Ctrl+C to exit." << "\n";

	// Start the server
	server.start(); // starts the server's main loop which will accept incoming connections, handle each connection, continue until interrupted

	return 0;
}

int main() {
	Response resp;
	resp.setStatusCode(200);
	resp.setReasonPhrase("OK");

	resp.addHeader("Content-Type", "text/plain");
	resp.addHeader("Connection", "close");

	std::string msg = "Hello from Webserv!\n";
	std::vector<char> body(msg.begin(), msg.end());
	resp.setBody(std::move(body));

	resp.addHeader("Content-Length", std::to_string(resp.getBody().size()));

	std::vector<char> out = resp.serialize();

	std::cout.write(out.data(), out.size());
	return 0;
}

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
