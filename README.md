# Webserv

A HTTP/1.1 compliant web server implementation in C++98.

## Building

```bash
make        # Build the server
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Rebuild everything
```

## Running

```bash
./webserv [config_file]  # Start server with specified config
./webserv               # Start server with default config
```

## Project Structure

- `src/`: Source files
- `config/`: Configuration files
- `www/`: Web root directory
- `tests/`: Test files
- `docs/`: Documentation

## Useful Resources

### HTTP Protocol

- [RFC 2616 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616)
- [MDN HTTP Documentation](https://developer.mozilla.org/en-US/docs/Web/HTTP)

### Socket Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Non-blocking I/O](https://www.gnu.org/software/libc/manual/html_node/Nonblocking-Read_002fWrite.html)

### CGI

- [RFC 3875 - CGI Version 1.1](https://datatracker.ietf.org/doc/html/rfc3875)
- [CGI Programming in C++](http://www.cgi101.com/book/ch1/text.html)

### Testing Tools

- `curl`: Testing HTTP requests
- `siege`: Load testing
- `valgrind`: Memory leak detection
- `telnet`: Raw HTTP communication

### Example HTTP Request (for testing)

```
GET / HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0
Accept: text/html
```

## Development Guidelines

1. Follow C++98 standard
2. Use non-blocking I/O with poll/select
3. Handle errors gracefully
4. Test against NGINX for behavior verification
5. Keep code modular and well-documented

## Testing

1. Basic HTTP requests (GET, POST, DELETE)
2. File uploads
3. CGI execution
4. Error handling
5. Load testing
6. Memory leak checking
