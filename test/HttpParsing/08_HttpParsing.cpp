#include <iostream>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_negative_content_length() {
    std::cout << "Test 8: Negative Content-Length... ";
    HttpParser parser;
    
    const char* request = 
        "POST /test HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: -100\r\n"
        "\r\n";
    
    try {
        parser.consume(request, strlen(request));
        std::cout << "FAILED (Should throw BAD_REQUEST)\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        if (e.e_status == BAD_REQUEST)
            std::cout << "PASSED\n";
        else
            std::cout << "FAILED (Wrong exception: " << e.e_status << ")\n";
    }
}
