/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:04:57 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/16 17:59:50 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../Client.hpp"
#include "../Request/Request.hpp"
#include "../ServerManager.hpp"
#include "../parsingConfFile/vServer.hpp"
#include "../parsingConfFile/Location.hpp"

class Response {
    private:
        Request *_request;
        ServerManager *_serverManager; // Server manager to access server configurations
        vServer *_serverConfig; // virtual server configuration for the response
        Location *_locationConfig; // Location configuration for the response
        int _clientSocket;
        int _statusCode; // HTTP status code (e.g., 200, 404, 500)
        std::string _rawResponse;
        std::string _statusMessage; // HTTP status message (e.g., "OK", "Not Found", "Internal Server Error")
        std::unordered_map<std::string, std::string> _headers; // HTTP headers for the response
        std::string _body; // Body of the response
        std::unordered_map<int, std::string> _statusMessages = {
            {200, "OK"},
            {301, "Moved Permanently"},
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {405, "Method Not Allowed"},
            {408, "Request Timeout"},
            {418, "I'm a teapot"},
            {429, "Too Many Requests"},
            {500, "Internal Server Error"},
            {505, "HTTP Version Not Supported"}
        };

        void createStartLine(); // Create the start line of the HTTP response
        void createHeaders(); // Create the headers for the HTTP response
        void defaultHeaders(); // Set default headers for the response
        void createBody(); // Create the body of the HTTP response

        void handleGetRequest();
        void handlePostRequest();
        void handleDeleteRequest();
        void handleCGIRequest(); // private
        void handleRedirectRequest(); // private
        
        void makeRegularResponse(const std::string &path);
        void makeChunkedResponse(const std:: string &path);
        void generateErrorResponse(); // Generate an error response
        void matchServer(); // Match the server configuration based on the request
        void matchLocation(); // Match the location configuration based on the request
        bool isMethodAllowed(const std::string &method) const; // Check if the method is allowed in the location
        bool fileExists(const std::string &path); // Check if the file exists
        bool isLargeFile(const std::string &path); // Check if the file is larger than a certain threshold
        std::string generateDirectoryListing(const std::string& fsPath, const std::string& urlPath);


    public:
        Response();
        Response(Request *request, ServerManager *serverManager, int clientSocket);
        Response(const Response &src);
        Response &operator=(const Response &src);
        ~Response();

        //Status line
        int getStatusCode() const;
        void setStatusCode(int statusCode);
        const std::string& getStatusMessage() const;
        void setStatusMessage(const std::string& reasonPhrase);

        //Headers
        void addHeader(const std::string& key, const std::string& value);
        const std::unordered_map<std::string, std::string>& getHeaders() const;

        //Body
        void setBody(const std::string &body);
        const std::string &getBody() const;
        const std::string& getRawResponse() const;

        void generateResponse(); // Generate the full HTTP response string
};

#endif