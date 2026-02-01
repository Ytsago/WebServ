#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_empty_body_with_content_length_zero() {
    std::cout << "Test 4: Empty body (Content-Length: 0)... ";
    HttpParser parser;
    
    const char* request = 
        "POST /empty HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    
    try {
        parser.consume(request, strlen(request));
        assert(parser.isComplete());
        
        HttpRequest* req = parser.generateRequest();
        assert(req->getBody().empty());
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
