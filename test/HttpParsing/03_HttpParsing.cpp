#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_fragmented_parsing() {
    std::cout << "Test 3: Fragmented parsing... ";
    HttpParser parser;
    
    const char* part1 = "GET /test HTTP/1.1\r\n";
    const char* part2 = "Host: example.com\r\n";
    const char* part3 = "Content-Length: 5\r\n\r\n";
    const char* part4 = "12345";
    
    try {
        parser.consume(part1, strlen(part1));
        assert(!parser.isComplete());
        
        parser.consume(part2, strlen(part2));
        assert(!parser.isComplete());
        
        parser.consume(part3, strlen(part3));
        assert(!parser.isComplete());
        
        parser.consume(part4, strlen(part4));
        assert(parser.isComplete());
        
        HttpRequest* req = parser.generateRequest();
        assert(req->getBody().size() == 5);
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
