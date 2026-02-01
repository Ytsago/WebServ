#include <iostream>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_invalid_http_version() {
    std::cout << "Test 6: Invalid HTTP version... ";
    HttpParser parser;
    
    const char* request = 
        "GET /test HTTP/2.0\r\n"
        "Host: example.com\r\n"
        "\r\n";
    
    try {
        parser.consume(request, strlen(request));
        std::cout << "FAILED (Should throw HTTP_VERSION_NOT_SUPPORTED)\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        if (e.e_status == HTTP_VERSION_NOT_SUPPORTED)
            std::cout << "PASSED\n";
        else
            std::cout << "FAILED (Wrong exception: " << e.e_status << ")\n";
    }
}
