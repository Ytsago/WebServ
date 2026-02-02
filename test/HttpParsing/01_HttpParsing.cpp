#include <iostream>
#include <assert.h>
#include <cstring>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_basic_get_request() {
    std::cout << "Test 1: Basic GET request... ";
    HttpParser parser;
    
    const char* request = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: TestClient/1.0\r\n"
        "\r\n";
    
    try {
        parser.consume(request, strlen(request));
        assert(parser.isComplete());
        
        HttpRequest* req = parser.generateRequest();
        assert(req->getMethod() == "GET");
        assert(req->getUri() == "/index.html");
        assert(req->getBody().empty());
        delete req;
        std::cout << "PASSED\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        std::cout << "FAILED (Exception: " << e.e_status << ")\n";
    }
}
