/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amysiv <amysiv@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/01 21:02:06 by amysiv            #+#    #+#             */
/*   Updated: 2025/09/17 10:48:57 by amysiv           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP
#define REQUEST_READ_BUFFER 15000  ///< Maximum buffer size for reading client requests





#include "Request/Request.hpp"
#include "ServerManager.hpp"
#include "Server.hpp"
#include "Response/Response.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>





class ServerManager;
class Response;





/**
 * @class Client
 * @brief Represents a client connection handled by the server.
 *
 * The Client class stores request/response state, tracks activity,
 * and coordinates communication between the client socket and the server.
 */
class Client {
	private:
		Request _request;                          ///< Parsed client request object
		std::string _startLineAndHeadersBuffer;    ///< Buffer storing the start line and headers of the request
		std::string _bodyBuffer;                   ///< Buffer storing the request body
		bool _headersParsed;                       ///< Flag indicating if headers have been fully parsed
		size_t _bodyStart;                         ///< Position in buffer where the body begins

		size_t _clientBytesSent;                   ///< Number of bytes sent to the client so far
		std::string _clientResponse;               ///< Prepared response string for the client
		int _serverFd;                             ///< File descriptor of the client socket
		ServerManager* _serverManager;             ///< Pointer to the ServerManager managing this client
		std::unique_ptr<Response> _response;       ///< Response object being generated for this client
		std::time_t _lastActiveTime;               ///< Last time this client was active (for timeout handling)
		bool _closeAfterResponse;                  ///< Flag indicating whether the connection should close after sending response

		/**
		 * @brief Check if the request headers are complete.
		 * @param request Request string to evaluate.
		 * @return True if headers are complete, false otherwise.
		 */
		bool headersComplete(const std::string& request);

		/**
		 * @brief Check if the request body is complete.
		 * @param body Request body string.
		 * @return True if the body is complete, false otherwise.
		 */
		bool bodyComplete(const std::string& body) const;

	public:
		/**
		 * @brief Construct a new Client object.
		 * @param serverFd Socket file descriptor for this client connection.
		 * @param servManager Pointer to the ServerManager.
		 */
		Client(int serverFd, ServerManager* servManager);

		/**
		 * @brief Destroy the Client object, cleaning up resources.
		 */
		~Client();

		/// @brief Copy constructor (disabled).
		Client(const Client&);

		/// @brief Assignment operator is deleted to avoid copying Client state.
		Client& operator=(const Client&) = delete;

		// ----------------- Setters -----------------
		/**
		 * @brief Update the last active timestamp for this client.
		 * @param timeStamp New activity timestamp.
		 */
		void setLastActiveTime(std::time_t timeStamp);

		/**
		 * @brief Mark this client as closed or active.
		 * @param flag True if closed, false otherwise.
		 */
		void setIsClosed(bool flag);

		// ----------------- Getters -----------------
		/**
		 * @brief Get the last active timestamp of this client.
		 * @return Last active time as std::time_t.
		 */
		std::time_t getLastActiveTime(void) const;

		/**
		 * @brief Get the server socket file descriptor associated with this client.
		 * @return File descriptor as int.
		 */
		int getServerFd(void) const;

		/**
		 * @brief Get the number of bytes sent to the client so far.
		 * @return Reference to bytes sent counter.
		 */
		size_t& getClientsBytesSent(void);

		/**
		 * @brief Get a reference to the client's Request object.
		 * @return Reference to Request.
		 */
		Request& getRequest(void) { return (_request); }

		/**
		 * @brief Get a reference to the Response object being prepared.
		 * @return Reference to Response.
		 */
		Response& getResponse(void) { return (*_response); }

		// ----------------- Methods -----------------
		/**
		 * @brief Handle incoming client request.
		 * @param clientFd Client socket file descriptor.
		 */
		void handleRequest(int clientFd);

		/**
		 * @brief Handle sending of the server response to the client.
		 * @param clientFd Client socket file descriptor.
		 */
		void handleResponse(int clientFd);

		/**
		 * @brief Send a prepared response body to the client.
		 * @param responseBody Response content string.
		 * @param clientFd Client socket file descriptor.
		 */
		void sendResponse(std::string responseBody, int clientFd);

		/**
		 * @brief Prepare a response string for the client.
		 * @param clientFd Client socket file descriptor.
		 * @return Prepared response as string.
		 */
		std::string prepareResponse(int clientFd);

		/**
		 * @brief Get the CGI response based on a Request.
		 * @param request The Request object.
		 * @return CGI response string.
		 */
		std::string getCgiResponse(Request& request);

		/**
		 * @brief Retrieve a header value from the headers map.
		 * @param headers Map of headers.
		 * @param headerName Name of the header to retrieve.
		 * @return Header value if found, empty string otherwise.
		 */
		static std::string getAnyHeader(std::unordered_map<std::string, std::string> headers, std::string headerName);
};

#endif
