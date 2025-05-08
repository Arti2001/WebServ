/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Response.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/18 16:04:57 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/18 16:07:36 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
class Response {
    private:
        int _status_code;
        std::string _reason_phrase;
        std::map<std::string, std::string> _headers;
        std::vector<char> _body;

    public:
        Response();
        ~Response();

        //Status line
        int getStatusCode() const;
        void setStatusCode(int statusCode);
        const std::string& getReasonPhrase() const;
        void setReasonPhrase(const std::string& reasonPhrase);

        //Headers
        void addHeader(const std::string& key, const std::string& value);
        const std::map<std::string, std::string>& getHeaders() const;

        //Body
        void setBody(const std::vector<char>& body);
        void setBody(std::vector<char>&& body);
        const std::vector<char>& getBody() const;

        //Serialize status line, headers and body into raw bytes
        std::vector<char> serialize() const;
};

#endif