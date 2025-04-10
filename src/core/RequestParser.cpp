/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:46:52 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/10 16:31:08 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"
/*
Plan:
    +++Store incoming data. check

    +++Check if request is complete using \r\n\r\n or Content-Length.

    Extract and parse only when full request is received.

    +++Handle multiple requests in one recv() call.

    Reject malformed requests (400 Bad Request).
*/

std::string RequestParser::urlDecode(const std::string& encoded) {
    
    std::string result;
    
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int value;
            std::istringstream hex_stream(encoded.substr(i + 1, 2));
            if (hex_stream >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += encoded[i];
            }
        } else if (encoded[i] == '+') {
            result += ' ';
        } else {
            result += encoded[i];
        }
    }
    return result;
}

void RequestParser::parseUrlEncodedForm(const std::string& body, std::unordered_map<std::string, std::string>& form_data) {
    
    std::istringstream stream(body);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        size_t equal_pos = pair.find('=');
        if (equal_pos != std::string::npos) {
            std::string key = pair.substr(0, equal_pos);
            std::string value = pair.substr(equal_pos + 1);

            key = urlDecode(key);
            value = urlDecode(value);

            form_data[key] = value;
        }
    }
}

std::string RequestParser::extractContentDispositionParameterValue(const std::string& text, const std::string& parameter, size_t start_pos) {
    
    std::string value;
    
    std::string parameter_name = parameter + "=";
    size_t parameter_pos = text.find(parameter_name, start_pos);

    if (parameter_pos != std::string::npos) {
        parameter_pos += parameter_name.length();
        
        char quote = text[parameter_pos];
        // Handle quoted values
        if (quote == '"' || quote == '\'') {
            parameter_pos++;
            size_t end_pos = text.find(quote, parameter_pos);
            if (end_pos != std::string::npos) {
                value = text.substr(parameter_pos, end_pos - parameter_pos);
            }
            // Handle unquoted values
        } else {
            size_t end_pos = text.find_first_of(" \r\n;", parameter_pos);
            
            if (end_pos != std::string::npos) {
                value = text.substr(parameter_pos, end_pos - parameter_pos);
            } else {
                value = text.substr(parameter_pos);
            }
        }
    }
    return value;
}

std::pair<std::string, std::string> RequestParser::parseContentDisposition(const std::string& headers_text) {
    
    std::string name, filename;
    size_t cont_disp_pos = headers_text.find("Content-Disposition:");
    if (cont_disp_pos != std::string::npos) {
        name = extractContentDispositionParameterValue(headers_text, "name", cont_disp_pos);
        filename = extractContentDispositionParameterValue(headers_text, "filename", cont_disp_pos);
    }
    return {name, filename};
}

int RequestParser::parseMultipartFormData(const std::string& body, const std::string& boundary,
                                            std::unordered_map<std::string, std::string>& form_data,
                                            std::unordered_map<std::string, std::string>& files) {

    std::string full_boundary = "--" + boundary;
    std::string end_boundary = "--" + boundary + "--";

    if (body.empty()) {
        return 400;
    }

    size_t pos = body.find(full_boundary);
    size_t next_boundary_pos;
    
    if (pos == std::string::npos) {
        return 400;
    }

    pos += full_boundary.length();
    
    while ((next_boundary_pos = body.find(full_boundary, pos)) != std::string::npos) {
        std::string part = body.substr(pos, next_boundary_pos - pos);
        
        if (part.length() > 2 && part.substr(0, 2) == "\r\n") {
            part = part.substr(2);
        }

        size_t headers_end = part.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            pos = next_boundary_pos + full_boundary.length();
            continue;
        }

        std::string headers_text = part.substr(0, headers_end);
        std::string part_content = part.substr(headers_end + 4);

        //Parse Content-Disposition
        auto [name, filename] = parseContentDisposition(headers_text);
        
        if (!name.empty()) {
            if (!filename.empty()) {
                files[name] = part_content;
            } else {
                if (part_content.length() >= 2 && part_content.substr(part_content.length() - 2) == "\r\n") {
                    part_content = part_content.substr(0, part_content.length() - 2);
                }
                form_data[name] = part_content;
            }
        }

        pos = next_boundary_pos + full_boundary.length();

        if (body.compare(next_boundary_pos, end_boundary.length(), end_boundary) == 0) {
            break;
        }
    }
    return 200;
}

std::string RequestParser::extractboundary(const std::string& content_type) {
    std::string boundary;
    size_t boundary_pos = content_type.find("boundary=");

    if (boundary_pos != std::string::npos) {
        boundary = content_type.substr(boundary_pos + 9);

        if (boundary.front() == '"' && boundary.back() == '"') {
            boundary = boundary.substr(1, boundary.length() - 2);
        }
    }
    return boundary;
}

void RequestParser::parseBody(std::string& body, HTTPRequest& request)
{
    std::string content_type = request._headers["Content-Type"];

    if (content_type.find("multipart/form-data") != std::string::npos)
    {   
        std::string boundary = extractboundary(content_type);
        if (!boundary.empty())
        {
            std::unordered_map<std::string, std::string> form_data;
            std::unordered_map<std::string, std::string> files;

            int status = parseMultipartFormData(body, boundary, form_data, files);
            if (status != 200)
            {
                std::cerr << "Error: Failed to parse multipart/form-data: " << status << std::endl;
                return;
            }
            request._form_data = form_data;
            request._files = files;
        }
    }
    else if (content_type.find("application/x-www-form-urlencode") != std::string::npos)
    {
        std::unordered_map<std::string, std::string> form_data;
        parseUrlEncodedForm(body, form_data);
        request._form_data = form_data;
    }
    else
    {
        request._body = body;
    }
}

std::pair<std::string, std::string> RequestParser::parseHeaders(std::string line, std::pair<std::string, std::string>& header_parsed)
{
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos)
    {
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        if (!key.empty() && std::isspace(key[0]))
        {
            throw std::runtime_error("400 BadRequest: Header name cannot start with whitespace");
        }
        key.erase(key.find_last_not_of(" \r\n\t") + 1);
        value.erase(0, value.find_first_not_of(" \r\n\t"));
        value.erase(value.find_last_not_of(" \r\n\t") + 1);

        header_parsed.first = key;
        header_parsed.second = value;
    }
    return header_parsed;
}

void RequestParser::parseFirstLineAndHeaders(std::string full_request, HTTPRequest& request)
{
    std::istringstream stream(full_request);
    std::unordered_map<std::string, std::string> headers;
    
    std::string method, path, version;
    
    stream >> method >> path >> version;
    
    request._method = method;
    request._uri = path;
    request._version = version; 

    std::string line;
    std::pair<std::string, std::string> header_parsed;
    while(std::getline(stream, line) && !line.empty())
    {
        if (line == "\r" || line == "\n")
            continue ;
        header_parsed = parseHeaders(line, header_parsed);
        headers[header_parsed.first] = header_parsed.second;
    }
    request._headers = headers;
}

std::unordered_map<int , HTTPRequest>& RequestParser::handleIncomingRequest(int fd, const std::string& raw_data, std::unordered_map<int, HTTPRequest>& resultMap)
{
    _request_buffers[fd] += raw_data;
    
    while (true)
    {
        
        std::size_t pos = _request_buffers[fd].find("\r\n\r\n");
        if (pos == std::string::npos)
        {
            return resultMap; // Request is not complete yet
        }
        std::string firstLine_and_headers = _request_buffers[fd].substr(0, pos + 4);
        
        HTTPRequest request;
        parseFirstLineAndHeaders(firstLine_and_headers, request);
        
        int content_length = 0;
        if (request._headers.find("Content-Length") != request._headers.end())
        {
            content_length = std::stoi(request._headers["Content-Length"]);
        }
        
        std::size_t total_request_size = pos + 4 + content_length;
        if (total_request_size > _request_buffers[fd].size())
        {
            return resultMap; // Request is not complete yet
        }

        std::string full_request = _request_buffers[fd].substr(0, total_request_size);
        std::string body = full_request.substr(pos + 4);
        parseBody(body, request);
        
        resultMap[fd] = request;
        
        _request_buffers[fd].erase(0, total_request_size);
        if (_request_buffers[fd].empty())
        {
            break;
        }
    }
    return resultMap;    
}