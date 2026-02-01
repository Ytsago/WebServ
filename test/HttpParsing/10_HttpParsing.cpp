#include <iostream>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_header_without_colon() {
    std::cout << "Test 10: Header without colon... ";
    HttpParser parser;
    
    const char* request = 
        "GET /test HTTP/1.1\r\n"
        "InvalidHeader\r\n"
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
