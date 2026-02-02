#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_post_with_body() {
    std::cout << "Test 2: POST with body... ";
    HttpParser parser;
    
    const char* request = 
        "POST /api/data HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    
    try {
        parser.consume(request, strlen(request));
        assert(parser.isComplete());
        
        HttpRequest* req = parser.generateRequest();
        assert(req->getMethod() == "POST");
        assert(req->getBody().size() == 13);
        std::string body(req->getBody().begin(), req->getBody().end());
        assert(body == "Hello, World!");
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
