#include <iostream>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_body_too_large() {
    std::cout << "Test 9: Body exceeds MAX_BODY_SIZE... ";
    HttpParser parser;
    
    const char* request = 
        "POST /test HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 2000000\r\n"
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
