/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: pminialg <pminialg@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:46:52 by pminialg      #+#    #+#                 */
/*   Updated: 2025/04/12 14:06:24 by pminialg      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

void RequestParser::validateRequestLine(const std::string& method, const std::string& uri, const std::string& version) {
    if (method.empty() || uri.empty() || version.empty()) {
        throw std::runtime_error("400 BadRequest: Invalid request line");
    }

    if (method != "GET" && method != "POST" && method != "DELETE") {
        throw std::runtime_error("400 BadRequest: Invalid method");
    }

    if (version != "HTTP/1.1") {
        throw std::runtime_error("505 HTTP Version Not Supported");
    }
}

size_t RequestParser::skipLeadingEmptyLines(const std::string& data) {
    size_t pos = 0;

    while (pos < data.size()) {
        if (pos + 1 < data.size() && data[pos] == '\r' && data[pos + 1] == '\n') {
            pos += 2;
        } else if (data[pos] == '\n') {
            pos++;
        } else {
            break;
        }
    }
    return pos;
}

std::pair<size_t, size_t> RequestParser::findHeadersEnd(const std::string& data) {
    size_t pos_crlf = data.find("\r\n\r\n");
    size_t pos_lf = data.find("\n\n");

    if (pos_crlf == std::string::npos && pos_lf == std::string::npos) {
        return {std::string::npos, 0};
    }

    if (pos_crlf == std::string::npos) {
        return {pos_crlf, 4};
    }

    if (pos_lf == std::string::npos) {
        return {pos_lf, 2};
    }

    if (pos_crlf < pos_lf) {
        return {pos_crlf, 4};
    } else {
        return {pos_lf, 2};
    }
}

bool RequestParser::isValidHeaderFieldName(const std::string& name) {
    if (name.empty()) {
        return false;
    }

    for (char c : name) {
        if (std::isspace(c) || std::iscntrl(c) || c == ':') {
            return false;
        }
    }
    return true;
}

void RequestParser::stripLeadingWhitespace(std::string& text) {
    text.erase(0, text.find_first_not_of(" \r\n\t"));
}

int RequestParser::validateContentLength(const std::string& content) {
    for (char c : content) {
        if (!std::isdigit(c)) {
            return -1; // Invalid content-length
        }
    }
    try {
        int length = std::stoi(content);
        if (length < 0) {
            return -1; // Invalid content-length
        }
        return length;
    } catch (const std::exception& e) {
        return -1;
    }
}

std::string RequestParser::handleBareCR(const std::string& text) {
    
    std::string processed = text;
    size_t pos = 0;

    while ((pos = processed.find('\r', pos)) != std::string::npos) {
        if (pos + 1 >= processed.length() || processed[pos + 1] != '\n') {
            processed[pos] = ' ';
        }
        pos++;
    }
    return processed;
}

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
            headers_end = part.find("\n\n");
            if (headers_end == std::string::npos) {
                pos = next_boundary_pos + full_boundary.length();
                continue;
            }
        }

        std::string headers_text = part.substr(0, headers_end);
        std::string part_content;

        if (part.substr(headers_end + 4) == "\r\n\r\n") {
            part_content = part.substr(headers_end + 4);
        } else {
            part_content = part.substr(headers_end + 2);
        }

        //Parse Content-Disposition
        // fix some things below in this function
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

// std::pair<std::string, std::string> RequestParser::parseHeaders(std::string line, std::pair<std::string, std::string>& header_parsed)
// {
//     size_t colon_pos = line.find(':');
//     if (colon_pos != std::string::npos)
//     {
//         std::string key = line.substr(0, colon_pos);
//         std::string value = line.substr(colon_pos + 1);

//         if (!key.empty() && std::isspace(key[0]))
//         {
//             throw std::runtime_error("400 BadRequest: Header name cannot start with whitespace");
//         }
//         key.erase(key.find_last_not_of(" \r\n\t") + 1);
//         value.erase(0, value.find_first_not_of(" \r\n\t"));
//         value.erase(value.find_last_not_of(" \r\n\t") + 1);

//         header_parsed.first = key;
//         header_parsed.second = value;
//     }
//     return header_parsed;
// }

std::pair<std::string, std::string> RequestParser::parseHeader(const std::string& header_line) {
    
}

std::tuple<std::string, std::string, std::string> RequestParser::parseFirstLine(const std::string& first_line) {
    
    std::string method, uri, version;
    
    size_t first_space = first_line.find(' ');
    if (first_space == std::string::npos) {
        throw std::runtime_error("400 BadRequest: Invalid request line format (missing space)");
    }
    
    method = first_line.substr(0, first_space);

    size_t uri_start = first_space + 1;
    if (uri_start >= first_line.length() || first_line[uri_start] == ' ') {
        throw std::runtime_error("400 BadRequest: Invalid request line format (multiple spaces)");
    }

    size_t second_space = first_line.find(' ', uri_start);
    if (second_space == std::string::npos) {
        throw std::runtime_error("400 BadRequest: Invalid request line format (missing space)");
    }

    uri = first_line.substr(uri_start, second_space - uri_start);

    size_t version_start = second_space + 1;
    if (version_start >= first_line.length() || first_line[version_start] == ' ') {
        throw std::runtime_error("400 BadRequest: Invalid request line format (multiple spaces)");
    }

    size_t third_space = first_line.find(' ', version_start);
    if (third_space != std::string::npos) {
        throw std::runtime_error("400 BadRequest: Invalid request line format (extra content)");
    }

    version = first_line.substr(version_start);

    validateRequestLine(method, uri, version);

    return {method, uri, version};
}

void RequestParser::parseFirstLineAndHeaders(std::string firstLine_and_headers, HTTPRequest& request)
{
    std::istringstream stream(firstLine_and_headers);
    std::unordered_map<std::string, std::string> headers;

    std::string start;
    std::getline(stream, start);

    if (!start.empty() && start.back() == '\r') {
        start.pop_back();
    }
    
    auto [method, uri, version] = parseFirstLine(start);

    request._method = method;
    request._uri = uri;
    request._version = version;

    std::string line;
    bool first_header_line = true;
    
    while (std::getline(stream, line) && !line.empty()) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (first_header_line && !line.empty() && std::isspace(line[0])) {
            continue;
        }
        first_header_line = false;
        
        try {
            auto header = parseHeader(line);
            if (!header.first.empty()) {
                headers[header.first] = header.second;
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: " << e.what() << std::endl;
        }
    }

    request._headers = headers;
}

std::unordered_map<int , HTTPRequest>& RequestParser::handleIncomingRequest(int fd, const std::string& raw_data, std::unordered_map<int, HTTPRequest>& resultMap)
{
    _request_buffers[fd] += raw_data;
    
    while (!_request_buffers[fd].empty())
    {
        size_t pos_start = skipLeadingEmptyLines(_request_buffers[fd]);
        if (pos_start > 0)
        {
            _request_buffers[fd].erase(0, pos_start);
        }
        
        auto [headers_end_pos, headers_end_length] = findHeadersEnd(_request_buffers[fd]);

        if (headers_end_pos == std::string::npos)
            break;

        std::string firstLine_and_headers = _request_buffers[fd].substr(0, headers_end_pos + headers_end_length);

        HTTPRequest request;
        try
        {
            parseFirstLineAndHeaders(firstLine_and_headers, request);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error: Failed to parse request: " << e.what() << std::endl;
            _request_buffers[fd].erase(0, headers_end_pos + headers_end_length);
            continue;
        }
        
        int content_length = 0;
        if (request._headers.find("Content-Length") != request._headers.end())
        {
            int validated_content_length = validateContentLength(request._headers["Content-Length"]);
            if (validated_content_length < 0)
            {
                std::cerr << "Error: Invalid Content-Length: " << request._headers["Content-Length"] << std::endl;
                _request_buffers[fd].erase(0, headers_end_pos + headers_end_length);
                continue;
            }
            content_length = validated_content_length;
        }
        
        std::size_t total_request_size = headers_end_pos + headers_end_length + content_length;
        if (total_request_size > _request_buffers[fd].size())
        {
            break; // Request is not complete yet
        }

        std::string body = _request_buffers[fd].substr(headers_end_pos + headers_end_length, content_length);

        parseBody(body, request);

        resultMap[fd] = request;

        _request_buffers[fd].erase(0, total_request_size);
    }

    return resultMap;
}