#include "CGIHandler.hpp"
#include "../parsingRequest/HTTPRequest.hpp"
#include "../parsingResponse/Response.hpp"
#include <iostream>
#include <fstream>

// Function to save response to a file
void saveResponseToFile(const Response& response, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        const std::vector<char>& body = response.getBody();
        file.write(body.data(), body.size());
        file.close();
        std::cout << "Response saved to " << filename << std::endl;
    } else {
        std::cout << "Failed to open file: " << filename << std::endl;
    }
}

// Function to display response details
void printResponseDetails(const Response& response) {
    std::cout << "Status: " << response.getStatusCode() << " " << response.getReasonPhrase() << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto& header : response.getHeaders()) {
        std::cout << "  " << header.first << ": " << header.second << std::endl;
    }
    
    if (response.getStatusCode() >= 400) {
        // For error responses, show the body
        std::string errorMsg(response.getBody().begin(), response.getBody().end());
        std::cout << "Error: " << errorMsg << std::endl;
    } else if (!response.getBody().empty()) {
        // For successful responses, show size and preview
        std::cout << "Body size: " << response.getBody().size() << " bytes" << std::endl;
        std::cout << "Body preview: ";
        int previewSize = std::min(50, (int)response.getBody().size());
        std::string preview(response.getBody().begin(), response.getBody().begin() + previewSize);
        std::cout << preview << (response.getBody().size() > 50 ? "..." : "") << std::endl;
    }
}

int main() {
    // First, let's make sure our CGI scripts are executable
    std::cout << "Ensuring CGI scripts are executable..." << std::endl;
    system("chmod +x myWebsite/cgi-bin/*.py");
    
    // Create a simple shell script CGI test to rule out Python issues
    std::cout << "Creating a simple shell script for testing..." << std::endl;
    std::ofstream shellScript("myWebsite/cgi-bin/test.sh");
    shellScript << "#!/bin/sh\n";
    shellScript << "echo \"Content-Type: text/plain\"\n";
    shellScript << "echo \"\"\n";
    shellScript << "echo \"CGI Shell Script Test\"\n";
    shellScript << "echo \"Current directory: $(pwd)\"\n";
    shellScript << "echo \"Script name: $0\"\n";
    shellScript << "echo \"Environment variables:\"\n";
    shellScript << "env | sort\n";
    shellScript.close();
    system("chmod +x myWebsite/cgi-bin/test.sh");
    
    // Create CGI handler
    CGIHandler cgiHandler;
    
    std::cout << "\n=== Testing CGI Scripts ===" << std::endl;
    
    // Test shell script (simplest case)
    std::cout << "\n--- Testing shell script (GET) ---" << std::endl;
    HTTPRequest shellRequest;
    shellRequest.setMethod("GET");
    shellRequest.setUri("/cgi-bin/test.sh");
    shellRequest.setVersion("HTTP/1.1");
    
    try {
        Response shellResponse = cgiHandler.handle(shellRequest);
        printResponseDetails(shellResponse);
        saveResponseToFile(shellResponse, "shell_response.txt");
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    // Test 1: hello.py with GET request
    std::cout << "\n--- Testing hello.py (GET) ---" << std::endl;
    HTTPRequest helloRequest;
    helloRequest.setMethod("GET");
    helloRequest.setUri("/cgi-bin/hello.py");
    helloRequest.setVersion("HTTP/1.1");
    
    try {
        Response helloResponse = cgiHandler.handle(helloRequest);
        printResponseDetails(helloResponse);
        saveResponseToFile(helloResponse, "hello_response.html");
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    // Test 2: test.py with query parameters
    std::cout << "\n--- Testing test.py with query parameters ---" << std::endl;
    HTTPRequest testRequest;
    testRequest.setMethod("GET");
    testRequest.setUri("/cgi-bin/test.py?param1=value1&param2=value2");
    testRequest.setVersion("HTTP/1.1");
    
    try {
        Response testResponse = cgiHandler.handle(testRequest);
        printResponseDetails(testResponse);
        saveResponseToFile(testResponse, "test_response.html");
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Testing Complete ===" << std::endl;
    
    return 0;
}
