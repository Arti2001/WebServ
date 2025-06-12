/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: amysiv <amysiv@student.42.fr>                +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/02 10:46:52 by pminialg      #+#    #+#                 */
/*   Updated: 2025/06/12 14:24:08 by vovashko      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

RequestParser::RequestParser() {}

RequestParser::~RequestParser() {}

void RequestParser::validateRequestLine(const std::string &method, const std::string &uri, const std::string &version)
{
    if (method.empty() || uri.empty() || version.empty()) {
        throw std::runtime_error("400 Bad Request: Invalid request line");
    }
    if (method != "GET" && method != "POST" && method != "DELETE") {
        throw std::runtime_error("400 Bad Request: Invalid method");
    }
    if (version != "HTTP/1.1") {
        throw std::runtime_error("505 HTTP Version Not Supported");
    }
}

size_t RequestParser::skipLeadingEmptyLines(const std::string &data)
{
    size_t pos = 0;

    while (pos < data.size()) {
        if (pos + 1 < data.size() && data[pos] == '\r' && data[pos + 1] == '\n') {
            pos += 2;
        }
        else if (data[pos] == '\n') {
            pos++;
        }
        else {
            break;
        }
    }
    return pos;
}

bool RequestParser::isValidHeaderFieldName(const std::string &name)
{
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

int RequestParser::validateContentLength(const std::string &content_length)
{
    for (char c : content_length) {
        if (!std::isdigit(c)) {
            return -1; // Invalid content-length
        }
    }
    try {
        int length = std::stoi(content_length);
        if (length < 0) {
            return -1; // Invalid content-length
        }
        return length;
    }
    catch (const std::exception &e) {
        return -1;
    }
}

std::string RequestParser::handleBareCR(const std::string &text)
{

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

std::string RequestParser::urlDecode(const std::string &encoded)
{

    std::string result;

    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int value;
            std::istringstream hex_stream(encoded.substr(i + 1, 2));
            if (hex_stream >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            }
            else {
                result += encoded[i];
            }
        }
        else if (encoded[i] == '+') {
            result += ' ';
        }
        else {
            result += encoded[i];
        }
    }
    return result;
}

void RequestParser::parseUrlEncodedForm(const std::string &body, std::unordered_map<std::string, std::string> &form_data)
{

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

std::string RequestParser::extractContentDispositionParameterValue(const std::string &text, const std::string &parameter, size_t start_pos)
{

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
        }
        else {
            // Handle unquoted values
            size_t end_pos = text.find_first_of(" \r\n;", parameter_pos);

            if (end_pos != std::string::npos) {
                value = text.substr(parameter_pos, end_pos - parameter_pos);
            }
            else {
                value = text.substr(parameter_pos);
            }
        }
    }
    return value;
}

std::pair<std::string, std::string> RequestParser::parseContentDisposition(const std::string &headers_text)
{

    std::string name, filename;
    size_t cont_disp_pos = headers_text.find("Content-Disposition:");
    if (cont_disp_pos != std::string::npos) {
        name = extractContentDispositionParameterValue(headers_text, "name", cont_disp_pos);
        filename = extractContentDispositionParameterValue(headers_text, "filename", cont_disp_pos);
    }
    return {name, filename};
}

int RequestParser::parseMultipartFormData(const std::string &body, const std::string &boundary,
                                          std::unordered_map<std::string, std::string> &form_data,
                                          std::unordered_map<std::string, std::string> &files)
{

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
        }
        else {
            part_content = part.substr(headers_end + 2);
        }

        auto [name, filename] = parseContentDisposition(headers_text);

        if (!name.empty()) {
            if (!filename.empty()) {
                files[name] = part_content;
            }
            else {
                if (part_content.length() >= 2 && part_content.substr(part_content.length() - 2) == "\r\n") {
                    part_content = part_content.substr(0, part_content.length() - 2);
                }
                else if (!part_content.empty() && part_content.back() == '\n') {
                    part_content.pop_back();
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


std::string RequestParser::extractBoundary(const std::string &content_type)
{
    std::string boundary;
    size_t boundary_pos = content_type.find("boundary=");

    if (boundary_pos != std::string::npos) {
        boundary = content_type.substr(boundary_pos + 9);

        if (!boundary.empty() && boundary.front() == '"' && boundary.back() == '"') {
            boundary = boundary.substr(1, boundary.length() - 2);
        }
    }
    return boundary;
}

bool RequestParser::isChunkedTransfer(const HTTPRequest &request) const
{
    auto it = request._headers.find("Transfer-Encoding");
    return it != request._headers.end() && it->second.find("chunked") != std::string::npos;
}

std::string RequestParser::decodeChunkedData(const std::string &chunked_data)
{
    std::string decoded_data;
    size_t pos = 0;

    while (pos < chunked_data.length()) {
        size_t line_end = chunked_data.find("\r\n", pos);
        if (line_end == std::string::npos) {
            // incomplete chunk
            break;
        }

        //extract and convert the chunk size
        std::string chunk_size_hex = chunked_data.substr(pos, line_end - pos);
        size_t chunk_size = 0;
        std::istringstream(chunk_size_hex) >> std::hex >> chunk_size;

        // if chunk size == 0, we've reached the end of the body
        if (chunk_size == 0) {
            return decoded_data;
        }

        // position after the CRLF
        pos = line_end + 2;

        // check if we have enough data for this chunk
        if (pos + chunk_size + 2 > chunked_data.length()) {
            // not enough data
            break;
        }

        decoded_data.append(chunked_data.substr(pos, chunk_size));

        //move position past this chunk
        pos += chunk_size + 2;
    }
    throw std::runtime_error("400 Bad Request: Incomplete chunked transfer encoding");
}

void RequestParser::parseBody(std::string &body, HTTPRequest &request)
{
    // Always store the raw body for CGI or other handlers that might need it.
    request._body = body;

    auto it = request._headers.find("Content-Type");
    if (it == request._headers.end()) {
        return; // No content type, nothing more to parse.
    }

    std::string content_type = it->second;

    if (content_type.find("multipart/form-data") != std::string::npos) {
        std::string boundary = extractBoundary(content_type);
        if (!boundary.empty()) {
            std::unordered_map<std::string, std::string> form_data;
            std::unordered_map<std::string, std::string> files;
            int status = parseMultipartFormData(body, boundary, form_data, files);
            if (status != 200) {
                std::cerr << "Error: Failed to parse multipart/form-data: " << status << std::endl;
                return;
            }
            request._form_data = form_data;
            request._files = files;
        }
    }
    else if (content_type.find("application/x-www-form-urlencode") != std::string::npos) {
        std::unordered_map<std::string, std::string> form_data;
        parseUrlEncodedForm(body, form_data);
        request._form_data = form_data;
    }
}

std::pair<std::string, std::string> RequestParser::parseHeader(const std::string &header_line)
{
    std::string key_before = header_line.substr(0, header_line.find(':')); 

    std::pair<std::string, std::string> header;

    std::string header_processed = handleBareCR(header_line);

    size_t colon_pos = header_processed.find(':');
    if (colon_pos != std::string::npos) {
        std::string key = header_processed.substr(0, colon_pos);
        std::string value = header_processed.substr(colon_pos + 1);

        if (!key.empty() && std::isspace(key[0])) {
            std::cerr << "Warning: 400 BadRequest: Header name cannot start with whitespace" << std::endl;
            return {};
        }

        key.erase(key.find_last_not_of(" \r\n\t") + 1);
        value.erase(0, value.find_first_not_of(" \r\n\t"));
        value.erase(value.find_last_not_of(" \r\n\t") + 1);

        if (!isValidHeaderFieldName(key)) {
            std::cerr << "Warning: 400 Bad Request: Invalid header field name" << std::endl;
            return {};
        }

        header.first = key;
        header.second = value;
    }
    return header;
}

std::tuple<std::string, std::string, std::string> RequestParser::parseFirstLine(const std::string &first_line)
{

    std::string method, uri, version;

    size_t first_space = first_line.find(' ');
    if (first_space == std::string::npos) {
        throw std::runtime_error("400 Bad Request: Invalid request line format (missing space)");
    }

    method = first_line.substr(0, first_space);

    size_t uri_start = first_space + 1;
    if (uri_start >= first_line.length() || first_line[uri_start] == ' ') {
        throw std::runtime_error("400 Bad Request: Invalid request line format (multiple spaces)");
    }

    size_t second_space = first_line.find(' ', uri_start);
    if (second_space == std::string::npos) {
        throw std::runtime_error("400 Bad Request: Invalid request line format (missing space)");
    }

    uri = first_line.substr(uri_start, second_space - uri_start);

    size_t version_start = second_space + 1;
    if (version_start >= first_line.length() || first_line[version_start] == ' ') {
        throw std::runtime_error("400 Bad Request: Invalid request line format (multiple spaces)");
    }

    size_t third_space = first_line.find(' ', version_start);
    if (third_space != std::string::npos) {
        throw std::runtime_error("400 Bad Request: Invalid request line format (extra content)");
    }

    version = first_line.substr(version_start);

    validateRequestLine(method, uri, version);

    return {method, uri, version};
}

void RequestParser::parseFirstLineAndHeaders(std::string firstLine_and_headers, HTTPRequest &request)
{
    std::istringstream stream(firstLine_and_headers);
    std::unordered_map<std::string, std::string> headers;
    std::string line;

    // Get the first line (request line)
    std::getline(stream, line);
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    auto [method, uri, version] = parseFirstLine(line);
    request._method = method;
    request._uri = uri;
    request._version = version;

    std::string current_header_line;
    while (std::getline(stream, current_header_line)) {
        // Handle CRLF line endings
        if (!current_header_line.empty() && current_header_line.back() == '\r') {
            current_header_line.pop_back();
        }

        // Check for header unfolding (obsolete but good to handle)
        if (!stream.eof() && (stream.peek() == ' ' || stream.peek() == '\t')) {
            std::string next_part;
            std::getline(stream, next_part);
            if (!next_part.empty() && next_part.back() == '\r') {
                next_part.pop_back();
            }
            // Trim leading whitespace from the folded part
            next_part.erase(0, next_part.find_first_not_of(" \t"));
            current_header_line += next_part;
            continue; // Continue to accumulate more folded lines if they exist
        }
        
        if (current_header_line.empty()) {
            continue; // Skip empty lines between headers
        }

        auto header = parseHeader(current_header_line);
        if (!header.first.empty()) {
            headers[header.first] = header.second;
        }

        // Reset for the next header
        current_header_line.clear();
    }
    request._headers = headers;
}
//std::string& RequestParser::getHostHeader( void ) const {
    
//}

std::unordered_map<int, HTTPRequest>        RequestParser::handleIncomingRequest(int fd, const std::string &raw_data)
{
    std::unordered_map<int, HTTPRequest> resultMap;
    _request_buffers[fd] += raw_data;

    while (!_request_buffers[fd].empty()) {
        size_t pos_start = skipLeadingEmptyLines(_request_buffers[fd]);
        if (pos_start > 0) {
            _request_buffers[fd].erase(0, pos_start);
        }

        auto [headers_end_pos, headers_end_length] = findHeadersEnd(_request_buffers[fd]);

        if (headers_end_pos == std::string::npos) {
            break;
        }

        std::string firstLine_and_headers = _request_buffers[fd].substr(0, headers_end_pos + headers_end_length);

        HTTPRequest request;
        try {
            parseFirstLineAndHeaders(firstLine_and_headers, request);
        }
        catch (const std::exception &e) {
            std::cerr << "Error: Failed to parse request: " << e.what() << std::endl;
            _request_buffers[fd].erase(0, headers_end_pos + headers_end_length);
            continue;
        }
        if (isChunkedTransfer(request)) {
            std::string chunked_data = _request_buffers[fd].substr(headers_end_pos + headers_end_length);
            try {
                std::string decoded_body = decodeChunkedData(chunked_data);
                request._body = decoded_body;
                request._headers["Content-Length"] = std::to_string(decoded_body.length());
                resultMap[fd] = request;
                _request_buffers[fd].erase(0, headers_end_pos + headers_end_length + chunked_data.length());
                continue;
            }
            catch (const std::exception &e) {
                // not enough data to decode chunked data
                break;
            }
        }

        int content_length = 0;
        if (request._headers.find("Content-Length") != request._headers.end()) {
            int validated_content_length = validateContentLength(request._headers["Content-Length"]);
            if (validated_content_length < 0) {
                std::cerr << "Error: Invalid Content-Length: " << request._headers["Content-Length"] << std::endl;
                _request_buffers[fd].erase(0, headers_end_pos + headers_end_length);
                continue;
            }
            content_length = validated_content_length;
        }

        std::size_t total_request_size = headers_end_pos + headers_end_length + content_length;
        if (total_request_size > _request_buffers[fd].size()) {
            break; // Request is not complete yet
        }

        std::string body = _request_buffers[fd].substr(headers_end_pos + headers_end_length, content_length);

        parseBody(body, request);

        resultMap[fd] = request;

        _request_buffers[fd].erase(0, total_request_size);
    }

    return resultMap;
}