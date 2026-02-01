#include <iostream>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_missing_spaces_in_request_line() {
    std::cout << "Test 7: Malformed request line (missing space)... ";
    HttpParser parser;
    
    const char* request = "GET/testHTTP/1.1\r\n\r\n";
    
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
