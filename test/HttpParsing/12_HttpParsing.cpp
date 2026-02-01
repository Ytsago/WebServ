#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_multiple_requests() {
    std::cout << "Test 12: Multiple requests (pipeline)... ";
    HttpParser parser;
    
    const char* request1 = 
        "GET /first HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    
    const char* request2 = 
        "GET /second HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";
    
    try {
        parser.consume(request1, strlen(request1));
        assert(parser.isComplete());
        HttpRequest* req1 = parser.generateRequest();
        assert(req1->getUri() == "/first");
        delete req1;
        
        parser.consume(request2, strlen(request2));
        assert(parser.isComplete());
        HttpRequest* req2 = parser.generateRequest();
        assert(req2->getUri() == "/second");
        delete req2;
        
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
