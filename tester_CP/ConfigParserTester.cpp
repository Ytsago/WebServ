#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "ConfigException.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

class ConfigParserTester {
private:
    int testsPassed;
    int testsFailed;
    std::string testConfigDir;

    void printTestResult(const std::string& testName, bool passed) {
        if (passed) {
            std::cout << GREEN << "[PASS] " << RESET << testName << std::endl;
            testsPassed++;
        } else {
            std::cout << RED << "[FAIL] " << RESET << testName << std::endl;
            testsFailed++;
        }
    }

    void createConfigFile(const std::string& filename, const std::string& content) {
        std::string filepath = testConfigDir + "/" + filename;
        std::ofstream file(filepath.c_str());
        if (file.is_open()) {
            file << content;
            file.close();
        }
    }

    void removeConfigFile(const std::string& filename) {
        std::string filepath = testConfigDir + "/" + filename;
        unlink(filepath.c_str());
    }

public:
    ConfigParserTester() : testsPassed(0), testsFailed(0) {
        testConfigDir = "./test_configs";
        mkdir(testConfigDir.c_str(), 0755);
    }

    ~ConfigParserTester() {
        rmdir(testConfigDir.c_str());
    }

    // ========== TESTS DE FICHIERS VALIDES ==========

    void testBasicValidConfig() {
        std::cout << BLUE << "\n=== Testing Basic Valid Config ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    server_name test.com;\n"
            "    host localhost;\n"
            "    root /var/www;\n"
            "    index index.html;\n"
            "}\n";

        createConfigFile("basic_valid.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/basic_valid.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            
            printTestResult("Basic valid: parsed successfully", servers.size() == 1);
            printTestResult("Basic valid: port correct", servers[0].get_listen_port() == 8080);
            printTestResult("Basic valid: server_name correct", servers[0].get_server_name() == "test.com");
            printTestResult("Basic valid: host correct", servers[0].get_host() == "localhost");
            printTestResult("Basic valid: root correct", servers[0].get_root() == "/var/www");
            printTestResult("Basic valid: index correct", servers[0].get_index() == "index.html");
        } catch (const std::exception& e) {
            printTestResult("Basic valid: parsed successfully", false);
            std::cout << "  Error: " << e.what() << std::endl;
        }

        removeConfigFile("basic_valid.conf");
    }

    void testMultipleServers() {
        std::cout << BLUE << "\n=== Testing Multiple Servers ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    server_name server1.com;\n"
            "}\n"
            "\n"
            "server {\n"
            "    listen 8081;\n"
            "    server_name server2.com;\n"
            "}\n"
            "\n"
            "server {\n"
            "    listen 8082;\n"
            "    server_name server3.com;\n"
            "}\n";

        createConfigFile("multiple.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/multiple.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            
            printTestResult("Multiple servers: 3 servers parsed", servers.size() == 3);
            printTestResult("Multiple servers: server 1 port", servers[0].get_listen_port() == 8080);
            printTestResult("Multiple servers: server 2 port", servers[1].get_listen_port() == 8081);
            printTestResult("Multiple servers: server 3 port", servers[2].get_listen_port() == 8082);
        } catch (const std::exception& e) {
            printTestResult("Multiple servers: 3 servers parsed", false);
        }

        removeConfigFile("multiple.conf");
    }

    void testLocationsConfig() {
        std::cout << BLUE << "\n=== Testing Locations ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    server_name test.com;\n"
            "    root /var/www;\n"
            "    index index.html;\n"
            "\n"
            "    location / {\n"
            "        allow_methods GET POST;\n"
            "        autoindex on;\n"
            "    }\n"
            "\n"
            "    location /api {\n"
            "        allow_methods GET POST DELETE;\n"
            "        root /var/www/api;\n"
            "        autoindex off;\n"
            "    }\n"
            "}\n";

        createConfigFile("locations.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/locations.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            std::vector<LocationConfig> locations = servers[0].get_locations();
            
            printTestResult("Locations: 2 locations parsed", locations.size() == 2);
            printTestResult("Locations: location 1 path", locations[0].get_path() == "/");
            printTestResult("Locations: location 1 autoindex", locations[0].get_autoindex() == true);
            printTestResult("Locations: location 2 path", locations[1].get_path() == "/api");
            printTestResult("Locations: location 2 root", locations[1].get_root() == "/var/www/api");
            
            std::vector<std::string> methods = locations[1].get_methods();
            printTestResult("Locations: location 2 has 3 methods", methods.size() == 3);
        } catch (const std::exception& e) {
            printTestResult("Locations: 2 locations parsed", false);
            std::cout << "  Error: " << e.what() << std::endl;
        }

        removeConfigFile("locations.conf");
    }

    void testCGIConfig() {
        std::cout << BLUE << "\n=== Testing CGI Configuration ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "\n"
            "    location /cgi-bin {\n"
            "        cgi_ext .py;\n"
            "        cgi_path /usr/bin/python3;\n"
            "        allow_methods GET POST;\n"
            "    }\n"
            "}\n";

        createConfigFile("cgi.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/cgi.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            std::vector<LocationConfig> locations = servers[0].get_locations();
            
            printTestResult("CGI: location parsed", locations.size() == 1);
            printTestResult("CGI: is_cgi flag set", locations[0].is_cgi() == true);
            printTestResult("CGI: extension correct", locations[0].get_cgi_ext() == ".py");
            printTestResult("CGI: path correct", locations[0].get_cgi_path() == "/usr/bin/python3");
        } catch (const std::exception& e) {
            printTestResult("CGI: location parsed", false);
        }

        removeConfigFile("cgi.conf");
    }

    void testCommentsAndWhitespace() {
        std::cout << BLUE << "\n=== Testing Comments and Whitespace ===" << RESET << std::endl;

        std::string config = 
            "# This is a comment\n"
            "\n"
            "server {\n"
            "    # Comment inside server\n"
            "    listen    8080   ;   # Trailing comment\n"
            "    server_name   test.com  ;\n"
            "    \n"
            "    # Empty lines and comments\n"
            "    \n"
            "    root /var/www;\n"
            "}\n";

        createConfigFile("comments.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/comments.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            
            printTestResult("Comments: parsed successfully", servers.size() == 1);
            printTestResult("Comments: port correct", servers[0].get_listen_port() == 8080);
            printTestResult("Comments: server_name correct", servers[0].get_server_name() == "test.com");
        } catch (const std::exception& e) {
            printTestResult("Comments: parsed successfully", false);
        }

        removeConfigFile("comments.conf");
    }

    void testDefaultLocation() {
        std::cout << BLUE << "\n=== Testing Default Location ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    root /default/root;\n"
            "    index default_index.html;\n"
            "}\n";

        createConfigFile("default_loc.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/default_loc.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            
            printTestResult("Default location: is_default_set", servers[0].is_default_set() == true);
            
            LocationConfig defaultLoc = servers[0].get_default_location();
            printTestResult("Default location: root from server", defaultLoc.get_root() == "/default/root");
            printTestResult("Default location: index from server", defaultLoc.get_index() == "default_index.html");
        } catch (const std::exception& e) {
            printTestResult("Default location: is_default_set", false);
        }

        removeConfigFile("default_loc.conf");
    }

    void testClientMaxBodySize() {
        std::cout << BLUE << "\n=== Testing client_max_body_size ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    client_max_body_size 1048576;\n"
            "}\n";

        createConfigFile("body_size.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/body_size.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            
            printTestResult("Body size: correct value", servers[0].get_client_max_body_size() == 1048576);
        } catch (const std::exception& e) {
            printTestResult("Body size: correct value", false);
        }

        removeConfigFile("body_size.conf");
    }

    // ========== TESTS D'ERREURS ==========

    void testMissingClosingBrace() {
        std::cout << BLUE << "\n=== Testing Missing Closing Brace ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    server_name test.com;\n"
            // Missing }
            ;

        createConfigFile("missing_brace.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/missing_brace.conf").c_str());
            printTestResult("Missing brace: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("Missing brace: throws ConfigException", true);
            printTestResult("Missing brace: mentions missing brace", 
                          error.find("brace") != std::string::npos || error.find("Missing") != std::string::npos);
        } catch (...) {
            printTestResult("Missing brace: throws ConfigException", false);
        }

        removeConfigFile("missing_brace.conf");
    }

    void testInvalidPort() {
        std::cout << BLUE << "\n=== Testing Invalid Port ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen notanumber;\n"
            "}\n";

        createConfigFile("invalid_port.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/invalid_port.conf").c_str());
            printTestResult("Invalid port: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("Invalid port: throws ConfigException", true);
            printTestResult("Invalid port: mentions listen", error.find("listen") != std::string::npos);
        } catch (...) {
            printTestResult("Invalid port: throws ConfigException", false);
        }

        removeConfigFile("invalid_port.conf");
    }

    void testInvalidAutoindex() {
        std::cout << BLUE << "\n=== Testing Invalid Autoindex ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    location / {\n"
            "        autoindex maybe;\n"
            "    }\n"
            "}\n";

        createConfigFile("invalid_autoindex.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/invalid_autoindex.conf").c_str());
            printTestResult("Invalid autoindex: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("Invalid autoindex: throws ConfigException", true);
            printTestResult("Invalid autoindex: mentions autoindex", error.find("autoindex") != std::string::npos);
        } catch (...) {
            printTestResult("Invalid autoindex: throws ConfigException", false);
        }

        removeConfigFile("invalid_autoindex.conf");
    }

    void testInvalidMethod() {
        std::cout << BLUE << "\n=== Testing Invalid Method ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    location / {\n"
            "        allow_methods GET PATCH PUT;\n"
            "    }\n"
            "}\n";

        createConfigFile("invalid_method.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/invalid_method.conf").c_str());
            printTestResult("Invalid method: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("Invalid method: throws ConfigException", true);
            printTestResult("Invalid method: mentions method", error.find("method") != std::string::npos);
        } catch (...) {
            printTestResult("Invalid method: throws ConfigException", false);
        }

        removeConfigFile("invalid_method.conf");
    }

    void testIncompleteCGI() {
        std::cout << BLUE << "\n=== Testing Incomplete CGI Config ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    location /cgi {\n"
            "        cgi_ext .py;\n"
            "        # Missing cgi_path\n"
            "    }\n"
            "}\n";

        createConfigFile("incomplete_cgi.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/incomplete_cgi.conf").c_str());
            printTestResult("Incomplete CGI: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("Incomplete CGI: throws ConfigException", true);
            printTestResult("Incomplete CGI: mentions CGI or incomplete", 
                          error.find("CGI") != std::string::npos || error.find("incomplete") != std::string::npos);
        } catch (...) {
            printTestResult("Incomplete CGI: throws ConfigException", false);
        }

        removeConfigFile("incomplete_cgi.conf");
    }

    void testNestedServerBlock() {
        std::cout << BLUE << "\n=== Testing Nested Server Block ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    server {\n"  // Nested server - invalid
            "        listen 8081;\n"
            "    }\n"
            "}\n";

        createConfigFile("nested_server.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/nested_server.conf").c_str());
            printTestResult("Nested server: should throw exception", false);
        } catch (const ConfigException& e) {
            printTestResult("Nested server: throws ConfigException", true);
        } catch (...) {
            printTestResult("Nested server: throws ConfigException", false);
        }

        removeConfigFile("nested_server.conf");
    }

    void testNestedLocationBlock() {
        std::cout << BLUE << "\n=== Testing Nested Location Block ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    location / {\n"
            "        allow_methods GET;\n"
            "        location /nested {\n"  // Nested location - invalid
            "            allow_methods POST;\n"
            "        }\n"
            "    }\n"
            "}\n";

        createConfigFile("nested_location.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/nested_location.conf").c_str());
            printTestResult("Nested location: should throw exception", false);
        } catch (const ConfigException& e) {
            printTestResult("Nested location: throws ConfigException", true);
        } catch (...) {
            printTestResult("Nested location: throws ConfigException", false);
        }

        removeConfigFile("nested_location.conf");
    }

    void testMissingLocationPath() {
        std::cout << BLUE << "\n=== Testing Missing Location Path ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    location {\n"  // Missing path
            "        allow_methods GET;\n"
            "    }\n"
            "}\n";

        createConfigFile("missing_path.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/missing_path.conf").c_str());
            printTestResult("Missing location path: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("Missing location path: throws ConfigException", true);
            printTestResult("Missing location path: mentions path", error.find("path") != std::string::npos);
        } catch (...) {
            printTestResult("Missing location path: throws ConfigException", false);
        }

        removeConfigFile("missing_path.conf");
    }

    void testFileNotFound() {
        std::cout << BLUE << "\n=== Testing File Not Found ===" << RESET << std::endl;

        try {
            ConfigParser parser((testConfigDir + "/nonexistent.conf").c_str());
            printTestResult("File not found: should throw exception", false);
        } catch (const ConfigException& e) {
            std::string error(e.what());
            printTestResult("File not found: throws ConfigException", true);
            printTestResult("File not found: mentions file", 
                          error.find("file") != std::string::npos || error.find("open") != std::string::npos);
        } catch (...) {
            printTestResult("File not found: throws ConfigException", false);
        }
    }

    void testEmptyFile() {
        std::cout << BLUE << "\n=== Testing Empty File ===" << RESET << std::endl;

        createConfigFile("empty.conf", "");

        try {
            ConfigParser parser((testConfigDir + "/empty.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            printTestResult("Empty file: no servers", servers.size() == 0);
        } catch (const std::exception& e) {
            printTestResult("Empty file: no servers", false);
        }

        removeConfigFile("empty.conf");
    }

    void testOnlyComments() {
        std::cout << BLUE << "\n=== Testing File With Only Comments ===" << RESET << std::endl;

        std::string config = 
            "# Comment 1\n"
            "# Comment 2\n"
            "# Comment 3\n";

        createConfigFile("only_comments.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/only_comments.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            printTestResult("Only comments: no servers", servers.size() == 0);
        } catch (const std::exception& e) {
            printTestResult("Only comments: no servers", false);
        }

        removeConfigFile("only_comments.conf");
    }

    void testInvalidClientMaxBodySize() {
        std::cout << BLUE << "\n=== Testing Invalid client_max_body_size ===" << RESET << std::endl;

        std::string config = 
            "server {\n"
            "    listen 8080;\n"
            "    client_max_body_size invalid;\n"
            "}\n";

        createConfigFile("invalid_body_size.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/invalid_body_size.conf").c_str());
            printTestResult("Invalid body size: should throw exception", false);
        } catch (const ConfigException& e) {
            printTestResult("Invalid body size: throws ConfigException", true);
        } catch (...) {
            printTestResult("Invalid body size: throws ConfigException", false);
        }

        removeConfigFile("invalid_body_size.conf");
    }

    void testComplexRealWorldConfig() {
        std::cout << BLUE << "\n=== Testing Complex Real-World Config ===" << RESET << std::endl;

        std::string config = 
            "# Production server\n"
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com www.example.com;\n"
            "    host 0.0.0.0;\n"
            "    root /var/www/html;\n"
            "    index index.html index.htm;\n"
            "    error_page /errors/404.html;\n"
            "    client_max_body_size 10485760;\n"
            "\n"
            "    location / {\n"
            "        allow_methods GET POST;\n"
            "        autoindex off;\n"
            "    }\n"
            "\n"
            "    location /api {\n"
            "        allow_methods GET POST DELETE;\n"
            "        root /var/www/api;\n"
            "        autoindex off;\n"
            "    }\n"
            "\n"
            "    location /uploads {\n"
            "        allow_methods POST DELETE;\n"
            "        root /var/www/uploads;\n"
            "    }\n"
            "\n"
            "    location /cgi-bin {\n"
            "        allow_methods GET POST;\n"
            "        cgi_ext .py;\n"
            "        cgi_path /usr/bin/python3;\n"
            "    }\n"
            "}\n"
            "\n"
            "# Development server\n"
            "server {\n"
            "    listen 8080;\n"
            "    server_name dev.example.com;\n"
            "    host localhost;\n"
            "    root /var/www/dev;\n"
            "    index index.html;\n"
            "    client_max_body_size 52428800;\n"
            "\n"
            "    location / {\n"
            "        allow_methods GET POST DELETE;\n"
            "        autoindex on;\n"
            "    }\n"
            "}\n";

        createConfigFile("complex.conf", config);

        try {
            ConfigParser parser((testConfigDir + "/complex.conf").c_str());
            std::vector<ServerConfig> servers = parser.get_servers();
            
            printTestResult("Complex config: 2 servers", servers.size() == 2);
            printTestResult("Complex config: server 1 has 4 locations", 
                          servers[0].get_locations().size() == 4);
            printTestResult("Complex config: server 2 port", servers[1].get_listen_port() == 8080);
            printTestResult("Complex config: server 1 max body size", 
                          servers[0].get_client_max_body_size() == 10485760);
        } catch (const std::exception& e) {
            printTestResult("Complex config: 2 servers", false);
            std::cout << "  Error: " << e.what() << std::endl;
        }

        removeConfigFile("complex.conf");
    }

    void runAllTests() {
        std::cout << YELLOW << "\n╔════════════════════════════════════════╗" << RESET << std::endl;
        std::cout << YELLOW << "║   ConfigParser Complete Test Suite    ║" << RESET << std::endl;
        std::cout << YELLOW << "╚════════════════════════════════════════╝" << RESET << std::endl;

        // Tests de configurations valides
        testBasicValidConfig();
        testMultipleServers();
        testLocationsConfig();
        testCGIConfig();
        testCommentsAndWhitespace();
        testDefaultLocation();
        testClientMaxBodySize();
        testComplexRealWorldConfig();

        // Tests d'erreurs
        std::cout << YELLOW << "\n--- Error Handling Tests ---" << RESET << std::endl;
        testMissingClosingBrace();
        testInvalidPort();
        testInvalidAutoindex();
        testInvalidMethod();
        testIncompleteCGI();
        testNestedServerBlock();
        testNestedLocationBlock();
        testMissingLocationPath();
        testFileNotFound();
        testEmptyFile();
        testOnlyComments();
        testInvalidClientMaxBodySize();

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
    ConfigParserTester tester;
    tester.runAllTests();
    return 0;
}
