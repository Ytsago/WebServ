#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_case_insensitive_headers() {
    std::cout << "Test 15: Case-insensitive header keys... ";
    HttpParser parser;
    
    const char* request = 
        "GET /test HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/json\r\n"
        "CONTENT-LENGTH: 5\r\n"
        "\r\n"
        "12345";
    
    try {
        parser.consume(request, strlen(request));
        assert(parser.isComplete());
        
        std::string ct = parser.getHeader("content-type");
        assert(!ct.empty());
        
        HttpRequest* req = parser.generateRequest();
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
