#include <iostream>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_chunked_encoding() {
    std::cout << "Test 11: Chunked encoding (not implemented)... ";
    HttpParser parser;
    
    const char* request = 
        "POST /test HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    
    try {
        parser.consume(request, strlen(request));
        std::cout << "FAILED (Should throw NOT_IMPLEMENTED)\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        if (e.e_status == NOT_IMPLEMENTED)
            std::cout << "PASSED\n";
        else
            std::cout << "FAILED (Wrong exception: " << e.e_status << ")\n";
    }
}
