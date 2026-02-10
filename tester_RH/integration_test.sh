#!/bin/bash

# Script de test d'intégration pour RequestHandler
# Teste le comportement avec de vrais fichiers et répertoires

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TEST_DIR="/tmp/webserv_test"
PASSED=0
FAILED=0

setup_test_environment() {
    echo -e "${BLUE}Setting up test environment...${NC}"
    
    mkdir -p "$TEST_DIR/www"
    mkdir -p "$TEST_DIR/www/api"
    mkdir -p "$TEST_DIR/www/static"
    mkdir -p "$TEST_DIR/www/cgi-bin"
    mkdir -p "$TEST_DIR/www/uploads"
    
    echo "<html><body>Index Page</body></html>" > "$TEST_DIR/www/index.html"
    echo "<html><body>About Page</body></html>" > "$TEST_DIR/www/about.html"
    echo "<html><body>API Index</body></html>" > "$TEST_DIR/www/api/index.html"
    echo "<?php echo 'Hello PHP'; ?>" > "$TEST_DIR/www/cgi-bin/test.php"
    echo "#!/usr/bin/python3\nprint('Hello Python')" > "$TEST_DIR/www/cgi-bin/test.py"
    chmod +x "$TEST_DIR/www/cgi-bin/test.py"
    
    echo "body { color: red; }" > "$TEST_DIR/www/static/style.css"
    echo "console.log('test');" > "$TEST_DIR/www/static/script.js"
    
    echo -e "${GREEN}Test environment ready at $TEST_DIR${NC}\n"
}

cleanup_test_environment() {
    echo -e "\n${BLUE}Cleaning up test environment...${NC}"
    rm -rf "$TEST_DIR"
    echo -e "${GREEN}Cleanup complete${NC}"
}

print_test_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} $2"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} $2"
        ((FAILED++))
    fi
}

test_file_existence() {
    echo -e "\n${YELLOW}=== Testing File Existence ===${NC}"
    
    if [ -f "$TEST_DIR/www/index.html" ]; then
        print_test_result 0 "index.html exists"
    else
        print_test_result 1 "index.html exists"
    fi
    
    if [ -f "$TEST_DIR/www/cgi-bin/test.php" ]; then
        print_test_result 0 "CGI PHP script exists"
    else
        print_test_result 1 "CGI PHP script exists"
    fi
    
    if [ -x "$TEST_DIR/www/cgi-bin/test.py" ]; then
        print_test_result 0 "CGI Python script is executable"
    else
        print_test_result 1 "CGI Python script is executable"
    fi
}

test_directory_structure() {
    echo -e "\n${YELLOW}=== Testing Directory Structure ===${NC}"
    
    if [ -d "$TEST_DIR/www/api" ]; then
        print_test_result 0 "API directory exists"
    else
        print_test_result 1 "API directory exists"
    fi
    
    if [ -d "$TEST_DIR/www/static" ]; then
        print_test_result 0 "Static directory exists"
    else
        print_test_result 1 "Static directory exists"
    fi
    
    if [ -d "$TEST_DIR/www/uploads" ]; then
        print_test_result 0 "Uploads directory exists"
    else
        print_test_result 1 "Uploads directory exists"
    fi
}

test_file_permissions() {
    echo -e "\n${YELLOW}=== Testing File Permissions ===${NC}"
    
    if [ -r "$TEST_DIR/www/index.html" ]; then
        print_test_result 0 "index.html is readable"
    else
        print_test_result 1 "index.html is readable"
    fi
    
    if [ -w "$TEST_DIR/www/uploads" ]; then
        print_test_result 0 "uploads directory is writable"
    else
        print_test_result 1 "uploads directory is writable"
    fi
}

create_config_file() {
    echo -e "\n${YELLOW}=== Creating Test Config File ===${NC}"
    
    cat > "$TEST_DIR/test.conf" << EOF
server {
    listen 8080;
    server_name localhost;
    root $TEST_DIR/www;
    index index.html;
    client_max_body_size 1M;
    
    location / {
        methods GET POST DELETE;
        autoindex off;
    }
    
    location /api {
        root $TEST_DIR/www/api;
        methods GET POST;
    }
    
    location /static {
        root $TEST_DIR/www/static;
        methods GET;
    }
    
    location /cgi-bin {
        root $TEST_DIR/www/cgi-bin;
        methods GET POST;
        cgi_ext .php;
        cgi_path /usr/bin/php-cgi;
    }
    
    location /uploads {
        root $TEST_DIR/www/uploads;
        methods GET POST DELETE;
    }
}
EOF
    
    if [ -f "$TEST_DIR/test.conf" ]; then
        print_test_result 0 "Config file created"
        echo -e "${BLUE}Config file location: $TEST_DIR/test.conf${NC}"
    else
        print_test_result 1 "Config file created"
    fi
}

print_summary() {
    echo -e "\n${YELLOW}╔════════════════════════════════════════╗${NC}"
    echo -e "${YELLOW}║         Integration Test Summary       ║${NC}"
    echo -e "${YELLOW}╚════════════════════════════════════════╝${NC}"
    echo -e "${GREEN}Tests Passed: $PASSED${NC}"
    echo -e "${RED}Tests Failed: $FAILED${NC}"
    echo "Total Tests: $((PASSED + FAILED))"
    
    if [ $FAILED -eq 0 ]; then
        echo -e "\n${GREEN}✓ All integration tests passed!${NC}"
        return 0
    else
        echo -e "\n${RED}✗ Some integration tests failed!${NC}"
        return 1
    fi
}

print_usage_info() {
    echo -e "\n${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║         How to Use This Setup          ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
    echo ""
    echo "1. Test environment is at: $TEST_DIR"
    echo "2. Config file is at: $TEST_DIR/test.conf"
    echo "3. To compile your tester:"
    echo "   make -f Makefile_test"
    echo "4. To run the unit tests:"
    echo "   ./request_handler_test"
    echo "5. To test with your actual webserver:"
    echo "   ./webserv $TEST_DIR/test.conf"
    echo ""
    echo -e "${YELLOW}Test URLs you can try:${NC}"
    echo "  GET  http://localhost:8080/"
    echo "  GET  http://localhost:8080/about.html"
    echo "  GET  http://localhost:8080/api/"
    echo "  GET  http://localhost:8080/static/style.css"
    echo "  GET  http://localhost:8080/cgi-bin/test.php"
    echo "  POST http://localhost:8080/uploads/"
    echo ""
}

main() {
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║   RequestHandler Integration Tests    ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
    
    setup_test_environment
    test_file_existence
    test_directory_structure
    test_file_permissions
    create_config_file
    
    print_summary
    result=$?
    
    print_usage_info
    
    echo -e "\n${YELLOW}Note: Run 'bash $0 cleanup' to remove test files${NC}"
    
    return $result
}

if [ "$1" = "cleanup" ]; then
    cleanup_test_environment
else
    main
fi
