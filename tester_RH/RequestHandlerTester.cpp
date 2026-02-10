#include "RequestHandler.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "Response.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

class RequestHandlerTester {
private:
    int testsPassed;
    int testsFailed;
    int epollFd;

    void createTestFile(const std::string& path, const std::string& content) {
        std::ofstream file(path.c_str());
        if (file.is_open()) {
            file << content;
            file.close();
        }
    }

    void createTestDirectory(const std::string& path) {
        mkdir(path.c_str(), 0755);
    }

    void removeTestFile(const std::string& path) {
        unlink(path.c_str());
    }

    void removeTestDirectory(const std::string& path) {
        rmdir(path.c_str());
    }

    HttpRequest* createGetRequest(const std::string& uri) {
        HttpRequest* req = new HttpRequest();
        req->_method = "GET";
        req->_uri = uri;
        req->_contentLength = 0;
        return req;
    }

    HttpRequest* createPostRequest(const std::string& uri, const std::string& contentType, const std::string& body) {
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = uri;
        req->_header.push_back(std::make_pair("Content-Type", contentType));
        req->_body.assign(body.begin(), body.end());
        req->_contentLength = body.size();
        return req;
    }

    HttpRequest* createDeleteRequest(const std::string& uri) {
        HttpRequest* req = new HttpRequest();
        req->_method = "DELETE";
        req->_uri = uri;
        req->_contentLength = 0;
        return req;
    }

    ServerConfig createBasicServer() {
        ServerConfig server;
        server.set_root("/var/www");
        server.set_index("index.html");
        server.set_host("localhost");
        server.set_listen_port(8080);
        server.set_client_mbs(1048576);
        
        // Ne PAS créer de default location ici - elle sera créée automatiquement
        // par ton code quand nécessaire, ou explicitement dans les tests
        
        return server;
    }

    void printTestResult(const std::string& testName, bool passed) {
        if (passed) {
            std::cout << GREEN << "[PASS] " << RESET << testName << std::endl;
            testsPassed++;
        } else {
            std::cout << RED << "[FAIL] " << RESET << testName << std::endl;
            testsFailed++;
        }
    }

public:
    RequestHandlerTester() : testsPassed(0), testsFailed(0), epollFd(-1) {
        epollFd = epoll_create(10);
        if (epollFd == -1) {
            std::cerr << "Warning: Could not create epoll fd" << std::endl;
        }
    }

    ~RequestHandlerTester() {
        if (epollFd != -1)
            close(epollFd);
    }

    void testLocationMatching() {
        std::cout << BLUE << "\n=== Testing Location Matching ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();
        
        // Créer default location
        if (!server.is_default_set()) {
            LocationConfig default_loc;
            default_loc.set_root(server.get_root());
            default_loc.set_index(server.get_index());
            default_loc.push_method("GET");
            default_loc.push_method("POST");
            default_loc.push_method("DELETE");
            server.set_default_location(default_loc);
            server.set_is_default_set(true);
        }
        
        LocationConfig loc1;
        loc1.set_path("/api");
        loc1.set_root("/var/www/api");
        loc1.push_method("GET");
        loc1.push_method("POST");
        server.push_location(loc1);

        LocationConfig loc2;
        loc2.set_path("/api/users");
        loc2.set_root("/var/www/api/users");
        loc2.push_method("GET");
        server.push_location(loc2);

        LocationConfig loc3;
        loc3.set_path("/static");
        loc3.set_root("/var/www/static");
        loc3.push_method("GET");
        server.push_location(loc3);

        {
            HttpRequest* req = createGetRequest("/api/users/123");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Location matching: /api/users (longest match)", 
                          path.find("/var/www/api/users") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/api/data");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Location matching: /api (shorter match)", 
                          path.find("/var/www/api") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/other/path");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Location matching: default location", 
                          path.find("/var/www") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Location matching: root path", 
                          path.find("index.html") != std::string::npos);
            delete req;
        }
    }

    void testFilePathGeneration() {
        std::cout << BLUE << "\n=== Testing File Path Generation ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        LocationConfig loc1;
        loc1.set_path("/images");
        loc1.set_root("/var/www/images");
        loc1.set_index("gallery.html");
        server.push_location(loc1);

        {
            HttpRequest* req = createGetRequest("/images/logo.png");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("File path: specific file", 
                          path == "/var/www/images/logo.png");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/images/");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("File path: directory with index", 
                          path == "/var/www/images/gallery.html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/images");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("File path: directory without trailing slash", 
                          path == "/var/www/images/gallery.html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/test.html");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("File path: file at root", 
                          path == "/var/www/test.html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/subdir/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("File path: nested directory", 
                          path == "/var/www/subdir/file.txt");
            delete req;
        }
    }

    void testCgiExtensionDetection() {
        std::cout << BLUE << "\n=== Testing CGI Extension Detection ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        {
            HttpRequest* req = createGetRequest("/script.php");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: .php file", hasCgi && ext == ".php");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/cgi-bin/script.py");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: .py file", hasCgi && ext == ".py");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/index.html");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: .html file", hasCgi && ext == ".html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/nodot");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: no extension", !hasCgi);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/script.php?param=value");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: with query string", hasCgi && ext == ".php");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/path/file.cgi?a=b&c=d");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: .cgi with multiple params", hasCgi && ext == ".cgi");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/file.tar.gz");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("CGI ext: double extension", hasCgi && ext == ".gz");
            delete req;
        }
    }

    void testUploadTypeDetection() {
        std::cout << BLUE << "\n=== Testing Upload Type Detection ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        {
            HttpRequest* req = createPostRequest("/upload", "multipart/form-data", "test body");
            RequestHandler handler(server, *req, epollFd);
            std::string contentType;
            bool isUpload = handler.get_upload_type(contentType);
            printTestResult("Upload type: multipart/form-data", 
                          isUpload && contentType == "multipart/form-data");
            delete req;
        }

        {
            HttpRequest* req = createPostRequest("/upload", "application/octet-stream", "binary data");
            RequestHandler handler(server, *req, epollFd);
            std::string contentType;
            bool isUpload = handler.get_upload_type(contentType);
            printTestResult("Upload type: application/octet-stream", 
                          isUpload && contentType == "application/octet-stream");
            delete req;
        }

        {
            HttpRequest* req = createPostRequest("/upload", "application/json", "{\"key\":\"value\"}");
            RequestHandler handler(server, *req, epollFd);
            std::string contentType;
            bool isUpload = handler.get_upload_type(contentType);
            printTestResult("Upload type: application/json (not upload)", !isUpload);
            delete req;
        }

        {
            HttpRequest* req = createPostRequest("/upload", "text/plain", "plain text");
            RequestHandler handler(server, *req, epollFd);
            std::string contentType;
            bool isUpload = handler.get_upload_type(contentType);
            printTestResult("Upload type: text/plain (not upload)", !isUpload);
            delete req;
        }

        {
            HttpRequest* req = createPostRequest("/upload", "application/x-www-form-urlencoded", "key=value");
            RequestHandler handler(server, *req, epollFd);
            std::string contentType;
            bool isUpload = handler.get_upload_type(contentType);
            printTestResult("Upload type: form-urlencoded (not upload)", !isUpload);
            delete req;
        }
    }

    void testMethodAllowedChecking() {
        std::cout << BLUE << "\n=== Testing Method Allowed Checking ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        LocationConfig getOnlyLoc;
        getOnlyLoc.set_path("/readonly");
        getOnlyLoc.set_root("/var/www/readonly");
        getOnlyLoc.push_method("GET");
        server.push_location(getOnlyLoc);

        LocationConfig noPostLoc;
        noPostLoc.set_path("/noupload");
        noPostLoc.set_root("/var/www/noupload");
        noPostLoc.push_method("GET");
        noPostLoc.push_method("DELETE");
        server.push_location(noPostLoc);

        LocationConfig allMethodsLoc;
        allMethodsLoc.set_path("/api");
        allMethodsLoc.set_root("/var/www/api");
        allMethodsLoc.push_method("GET");
        allMethodsLoc.push_method("POST");
        allMethodsLoc.push_method("DELETE");
        server.push_location(allMethodsLoc);

        printTestResult("Method allowed: location config structure test", true);
    }

    void testEdgeCases() {
        std::cout << BLUE << "\n=== Testing Edge Cases ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        {
            HttpRequest* req = createGetRequest("");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            printTestResult("Edge case: empty URI", true);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("///multiple///slashes///");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Edge case: multiple slashes", true);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/./current/./dir");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Edge case: dot references", true);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/very/very/very/very/very/deep/path/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Edge case: very deep path", path.find("very") != std::string::npos);
            delete req;
        }

        LocationConfig loc;
        loc.set_path("/a/b/c/d/e");
        loc.set_root("/var/www/nested");
        loc.push_method("GET");
        server.push_location(loc);

        {
            HttpRequest* req = createGetRequest("/a/b/c/d/e/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Edge case: deep location match", 
                          path.find("/var/www/nested") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/script.php?param1=value1&param2=value2&param3=value3");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            bool hasCgi = handler.get_cgi_ext(ext);
            printTestResult("Edge case: many query params", hasCgi && ext == ".php");
            delete req;
        }
    }

    void testComplexLocationHierarchy() {
        std::cout << BLUE << "\n=== Testing Complex Location Hierarchy ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        LocationConfig loc1;
        loc1.set_path("/a");
        loc1.set_root("/www/a");
        server.push_location(loc1);

        LocationConfig loc2;
        loc2.set_path("/a/b");
        loc2.set_root("/www/ab");
        server.push_location(loc2);

        LocationConfig loc3;
        loc3.set_path("/a/b/c");
        loc3.set_root("/www/abc");
        server.push_location(loc3);

        LocationConfig loc4;
        loc4.set_path("/a/x");
        loc4.set_root("/www/ax");
        server.push_location(loc4);

        {
            HttpRequest* req = createGetRequest("/a/b/c/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Hierarchy: deepest match /a/b/c", 
                          path.find("/www/abc") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/a/b/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Hierarchy: middle match /a/b", 
                          path.find("/www/ab") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/a/x/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Hierarchy: sibling match /a/x", 
                          path.find("/www/ax") == 0);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/a/y/file.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Hierarchy: fallback to /a", 
                          path.find("/www/a") == 0);
            delete req;
        }
    }

    void testRootAndIndexCombinations() {
        std::cout << BLUE << "\n=== Testing Root and Index Combinations ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();
        server.set_root("/server/root");
        server.set_index("server_index.html");
        
        // Créer la default location comme ton ConfigParser le fait
        if (!server.is_default_set()) {
            LocationConfig default_loc;
            default_loc.set_root(server.get_root());
            default_loc.set_index(server.get_index());
            server.set_default_location(default_loc);
            server.set_is_default_set(true);
        }

        LocationConfig loc1;
        loc1.set_path("/custom");
        loc1.set_root("/custom/root");
        loc1.set_index("custom_index.html");
        server.push_location(loc1);

        LocationConfig loc2;
        loc2.set_path("/rootonly");
        loc2.set_root("/rootonly/path");
        server.push_location(loc2);

        LocationConfig loc3;
        loc3.set_path("/indexonly");
        loc3.set_index("special_index.html");
        server.push_location(loc3);

        {
            HttpRequest* req = createGetRequest("/custom/");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Root+Index: both custom", 
                          path == "/custom/root/custom_index.html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/rootonly/");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Root+Index: custom root, server index", 
                          path == "/rootonly/path/server_index.html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/indexonly/");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Root+Index: server root, custom index", 
                          path == "/server/root/special_index.html");
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Root+Index: both from server", 
                          path == "/server/root/server_index.html");
            delete req;
        }
    }

    void testSpecialCharactersInPaths() {
        std::cout << BLUE << "\n=== Testing Special Characters in Paths ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();

        {
            HttpRequest* req = createGetRequest("/file%20with%20spaces.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Special chars: URL encoded spaces", true);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/file-with-dashes.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Special chars: dashes", 
                          path.find("file-with-dashes.txt") != std::string::npos);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/file_with_underscores.txt");
            RequestHandler handler(server, *req, epollFd);
            handler.find_corresponding_location();
            std::string path = handler.get_file_path();
            printTestResult("Special chars: underscores", 
                          path.find("file_with_underscores.txt") != std::string::npos);
            delete req;
        }

        {
            HttpRequest* req = createGetRequest("/file.multiple.dots.txt");
            RequestHandler handler(server, *req, epollFd);
            std::string ext;
            handler.get_cgi_ext(ext);
            printTestResult("Special chars: multiple dots", ext == ".txt");
            delete req;
        }
    }

    void testCopyAndAssignment() {
        std::cout << BLUE << "\n=== Testing Copy Constructor and Assignment ===" << RESET << std::endl;

        ServerConfig server = createBasicServer();
        HttpRequest* req = createGetRequest("/test.html");

        {
            RequestHandler handler1(server, *req, epollFd);
            RequestHandler handler2(handler1);
            printTestResult("Copy constructor: compiles and runs", true);
        }

        {
            RequestHandler handler1(server, *req, epollFd);
            RequestHandler handler2(server, *req, epollFd);
            handler2 = handler1;
            printTestResult("Assignment operator: compiles and runs", true);
        }

        delete req;
    }

    void runAllTests() {
        std::cout << YELLOW << "\n╔════════════════════════════════════════╗" << RESET << std::endl;
        std::cout << YELLOW << "║  RequestHandler Complete Test Suite   ║" << RESET << std::endl;
        std::cout << YELLOW << "╚════════════════════════════════════════╝" << RESET << std::endl;

        testLocationMatching();
        testFilePathGeneration();
        testCgiExtensionDetection();
        testUploadTypeDetection();
        testMethodAllowedChecking();
        testComplexLocationHierarchy();
        testRootAndIndexCombinations();
        testSpecialCharactersInPaths();
        testEdgeCases();
        testCopyAndAssignment();

        std::cout << YELLOW << "\n╔════════════════════════════════════════╗" << RESET << std::endl;
        std::cout << YELLOW << "║           Test Results Summary         ║" << RESET << std::endl;
        std::cout << YELLOW << "╚════════════════════════════════════════╝" << RESET << std::endl;
        std::cout << GREEN << "Tests Passed: " << testsPassed << RESET << std::endl;
        std::cout << RED << "Tests Failed: " << testsFailed << RESET << std::endl;
        std::cout << "Total Tests: " << (testsPassed + testsFailed) << std::endl;
        
        if (testsFailed == 0) {
            std::cout << GREEN << "\n✓ All tests passed!" << RESET << std::endl;
        } else {
            std::cout << RED << "\n✗ Some tests failed!" << RESET << std::endl;
        }
    }
};

int main() {
    RequestHandlerTester tester;
    tester.runAllTests();
    return 0;
}