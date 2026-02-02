#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_large_body_fragmented() {
    std::cout << "Test 14: Large body in fragments... ";
    HttpParser parser;
    
    const char* header = 
        "POST /test HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 1000\r\n"
        "\r\n";
    
    try {
        parser.consume(header, strlen(header));
        assert(!parser.isComplete());
        
        // Send body in 100-byte chunks
        char chunk[100];
        memset(chunk, 'X', 100);
        for (int i = 0; i < 10; i++) {
            parser.consume(chunk, 100);
        }
        
        assert(parser.isComplete());
        HttpRequest* req = parser.generateRequest();
        assert(req->getBody().size() == 1000);
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
