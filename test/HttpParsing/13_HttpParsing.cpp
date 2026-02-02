#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_header_value_trimming() {
    std::cout << "Test 13: Header value whitespace trimming... ";
    HttpParser parser;
    
    const char* request = 
        "GET /test HTTP/1.1\r\n"
        "Host:   example.com   \r\n"
        "User-Agent:     Mozilla/5.0     \r\n"
        "\r\n";
    
    try {
        parser.consume(request, strlen(request));
        assert(parser.isComplete());
        
        std::string host = parser.getHeader("host");
        assert(host == "example.com");
        
        HttpRequest* req = parser.generateRequest();
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
