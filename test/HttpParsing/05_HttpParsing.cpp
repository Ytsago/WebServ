#include <iostream>
#include <cstring>
#include <assert.h>
#include "HttpRequest.hpp"
#include "HttpParser.hpp"

void test_uri_too_long() {
    std::cout << "Test 5: URI too long... ";
    HttpParser parser;
    
    std::string longUri(3000, 'a');
    std::string request = "GET /" + longUri + " HTTP/1.1\r\n";
    
    try {
        parser.consume(request.c_str(), request.length());
        std::cout << "FAILED (Should throw URI_TOO_LONG)\n";
    } catch (HttpParser::HttpRequestParsingException& e) {
        if (e.e_status == URI_TOO_LONG)
            std::cout << "PASSED\n";
        else
            std::cout << "FAILED (Wrong exception: " << e.e_status << ")\n";
    }
}

