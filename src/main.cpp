/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: amysiv <amysiv@student.42.fr>                +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/21 10:03:46 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/10 16:51:16 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "core/Server.hpp"
#include "core/HTTPRequest.hpp"
#include "core/RequestParser.hpp"

// Server *g_server = nullptr;
// bool g_running = true;


// void signalHandler(int signum)
// {
// 	std::cout << "\nInterupt signal (" << signum << ") received." << "\n";
// 	g_running = false;
// 	if (g_server) {
// 		g_server->stop();
// 	}
// 	std::cout << "Server shutting down..." << "\n";
// 	exit(signum);
// }

// int main(void)
// {
// 	signal(SIGINT, signalHandler);
// 	signal(SIGTERM, signalHandler);
// 	signal(SIGQUIT, signalHandler);

// 	Server server("8080", "127.0.0.1");
// 	g_server = &server; // stores the servers address in our global pointer

// 	if (!server.init()) // calls init which sets up the socket, binds it, and starts listening
// 	{
// 		std::cerr << "Failed to initialize server" << "\n";
// 		return 1;
// 	}

// 	std::cout << "Server running on port 8080. Press Ctrl+C to exit." << "\n";

// 	// Start the server
// 	server.start(); // starts the server's main loop which will accept incoming connections, handle each connection, continue until interrupted

// 	return 0;
// }

int main(void)
{
	std::string request_encoded = 
    "GET / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 29\r\n"
    "\r\n"
    "username=john&password=secret\r\n";
    
	std::string request_json = 
    "GET / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 43\r\n"
    "\r\n"
    "{'username': 'john', 'password': 'secret'}\r\n";

    std::string request_multipart =
    "POST /upload HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: multipart/form-data; boundary=34b21\r\n"
    "Content-Length: 406\r\n"
    "\r\n"
    "--34b21\r\n"
    "Content-Disposition: form-data; name='text'\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Book\r\n"
    "--34b21\r\n"
    "Content-Disposition: form-data; name='file1'; filename='a.json'\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\r\n"
    "'title': 'Java 8 in Action',\r\n"
    "'author': 'Mario Fusco',\r\n"
    "'year': 2014\r\n"
    "}\r\n"
    "--34b21\r\n"
    "Content-Disposition: form-data; name='file2'; filename='a.html'\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<title> Available for download! </title>\r\n"
    "--34b21--\r\n";
    
    
    // Print the request as is
    // std::cout << request;
    std::unordered_map<int, HTTPRequest> resultMap;
    
    RequestParser parser;
    // HTTPRequest rrr;
    parser.handleIncomingRequest(1, request_encoded, resultMap);
    parser.handleIncomingRequest(2, request_json, resultMap);
    parser.handleIncomingRequest(3, request_multipart, resultMap);
// parser.handleIncomingRequest(1, request2, resultMap);
// resultMap = parser.handleIncomingRequest(1, request, resultMap);
    std::cout << resultMap[1] << std::endl;
    std::cout << "----------" << std::endl;
    std::cout << resultMap[2] << std::endl;
    std::cout << "----------" << std::endl;
    std::cout << resultMap[3] << std::endl;
}

// std::string request = 
// "GET / HTTP/1.1\r\n"
// "Host: localhost:8080\r\n"
// "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
// "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
// "Accept-Language: en-US,en;q=0.5\r\n"
// "Accept-Encoding: gzip, deflate, br, zstd\r\n"
// "Connection: keep-alive\r\n"
// "Upgrade-Insecure-Requests: 1\r\n"
// "Sec-Fetch-Dest: document\r\n"
// "Sec-Fetch-Mode: navigate\r\n"
// "Sec-Fetch-Site: none\r\n"
// "Sec-Fetch-User: ?1\r\n"
// "Priority: u=0, i\r\n"
// "\r\n"
// "GET /favicon HTTP/1.1\r\n"
// "Host: localhost:8080\r\n"
// "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
// "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
// "Accept-Language: en-US,en;q=0.5\r\n"
// "Accept-Encoding: gzip, deflate, br, zstd\r\n"
// "Connection: keep-alive\r\n"
// "Upgrade-Insecure-Requests: 1\r\n"
// "Sec-Fetch-Dest: document\r\n"
// "Sec-Fetch-Mode: navigate\r\n"
// "Sec-Fetch-Site: none\r\n"
// "Sec-Fetch-User: ?1\r\n"
// "Priority: u=0, i\r\n"
// "\r\n"; 
// std::string request = 
// "GET / HTTP/1.1\r\n"
// "Host: localhost:8080\r\n"
// "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
// "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
// "Accept-Language: en-US,en;q=0.5\r\n"
// "Accept-Encoding: gzip, deflate, br, zstd\r\n"
// "Connection: keep-alive\r\n"
// "Upgrade-Insecure-Requests: 1\r\n"
// "Sec-Fetch-Dest: document\r\n"
// "Sec-Fetch-Mode: navigate\r\n"
// "Sec-Fetch-Site: none\r\n"
// "Sec-Fetch-User: ?1\r\n"
// "Priority: u=0, i\r\n"
// "\r\n";