#include "FileHandler.hpp"
#include "HttpRequest.hpp"
#include "LocationConfig.hpp"
#include "HeaderMap.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

class FileHandlerTester {
private:
    int testsPassed;
    int testsFailed;
    std::string uploadDir;
    std::string testFilesDir;

    void createTestDirectory(const std::string& path) {
        mkdir(path.c_str(), 0755);
    }

    void removeTestDirectory(const std::string& path) {
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    std::string filepath = path + "/" + entry->d_name;
                    unlink(filepath.c_str());
                }
            }
            closedir(dir);
        }
        rmdir(path.c_str());
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

    HttpRequest* createMultipartRequest(const std::string& boundary, const std::string& filename, 
                                       const std::string& content) {
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        
        std::string contentType = "multipart/form-data; boundary=" + boundary;
        req->_header.add("Content-Type", contentType);
        
        // Construire le body multipart
        std::stringstream body;
        body << "--" << boundary << "\r\n";
        body << "Content-Disposition: form-data; name=\"file\"; filename=\"" << filename << "\"\r\n";
        body << "Content-Type: application/octet-stream\r\n";
        body << "\r\n";
        body << content;
        body << "\r\n--" << boundary << "--\r\n";
        
        std::string bodyStr = body.str();
        req->_body.assign(bodyStr.begin(), bodyStr.end());
        req->_contentLength = bodyStr.size();
        
        return req;
    }

    HttpRequest* createMultipleFilesRequest(const std::string& boundary) {
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        
        std::string contentType = "multipart/form-data; boundary=" + boundary;
        req->_header.add("Content-Type", contentType);
        
        std::stringstream body;
        
        // Premier fichier
        body << "--" << boundary << "\r\n";
        body << "Content-Disposition: form-data; name=\"file1\"; filename=\"test1.txt\"\r\n";
        body << "Content-Type: text/plain\r\n";
        body << "\r\n";
        body << "Content of file 1";
        body << "\r\n";
        
        // Deuxième fichier
        body << "--" << boundary << "\r\n";
        body << "Content-Disposition: form-data; name=\"file2\"; filename=\"test2.txt\"\r\n";
        body << "Content-Type: text/plain\r\n";
        body << "\r\n";
        body << "Content of file 2";
        body << "\r\n";
        
        // Troisième fichier
        body << "--" << boundary << "\r\n";
        body << "Content-Disposition: form-data; name=\"file3\"; filename=\"test3.txt\"\r\n";
        body << "Content-Type: text/plain\r\n";
        body << "\r\n";
        body << "Content of file 3";
        body << "\r\n--" << boundary << "--\r\n";
        
        std::string bodyStr = body.str();
        req->_body.assign(bodyStr.begin(), bodyStr.end());
        req->_contentLength = bodyStr.size();
        
        return req;
    }

    bool fileExists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

    std::string readFile(const std::string& path) {
        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    int countFilesInDirectory(const std::string& path) {
        int count = 0;
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    count++;
                }
            }
            closedir(dir);
        }
        return count;
    }

    std::string getFirstFileInDirectory(const std::string& path) {
        DIR* dir = opendir(path.c_str());
        std::string filename;
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    filename = entry->d_name;
                    break;
                }
            }
            closedir(dir);
        }
        return filename;
    }

public:
    FileHandlerTester() : testsPassed(0), testsFailed(0) {
        uploadDir = "./website/uploads";
        testFilesDir = "./test_files";
        createTestDirectory("./website");
        createTestDirectory(uploadDir);
        createTestDirectory(testFilesDir);
    }

    ~FileHandlerTester() {
        removeTestDirectory(uploadDir);
        removeTestDirectory(testFilesDir);
        rmdir("./website");
    }

    void testBasicSingleFileUpload() {
        std::cout << BLUE << "\n=== Testing Basic Single File Upload ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundary123", "test.txt", "Hello World!");
        std::string contentType = req->_header["Content-Type"]; // Accès direct
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Single file upload: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Single file upload: correct content", content == "Hello World!");
            printTestResult("Single file upload: sanitized filename", 
                          filename.find("test.txt") != std::string::npos);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testMultipleFilesUpload() {
        std::cout << BLUE << "\n=== Testing Multiple Files Upload ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = createMultipleFilesRequest("----WebKitFormBoundary456");
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Multiple files upload: 3 files created", fileCount == 3);
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testEmptyFileUpload() {
        std::cout << BLUE << "\n=== Testing Empty File Upload ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundary789", "empty.txt", "");
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Empty file upload: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Empty file upload: zero size", content.empty());
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testLargeFileUpload() {
        std::cout << BLUE << "\n=== Testing Large File Upload ===" << RESET << std::endl;

        LocationConfig location;
        
        std::string largeContent;
        for (int i = 0; i < 1024; i++) {
            largeContent += "0123456789";
        }
        
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundary999", "large.bin", largeContent);
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Large file upload: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Large file upload: correct size", content.size() == largeContent.size());
            printTestResult("Large file upload: content matches", content == largeContent);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testBinaryFileUpload() {
        std::cout << BLUE << "\n=== Testing Binary File Upload ===" << RESET << std::endl;

        LocationConfig location;
        
        std::string binaryContent;
        for (unsigned char c = 0; c < 255; c++) {
            binaryContent += c;
        }
        
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundaryBIN", "binary.dat", binaryContent);
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Binary file upload: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Binary file upload: correct size", content.size() == binaryContent.size());
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testFilenameWithPath() {
        std::cout << BLUE << "\n=== Testing Filename with Path (Security) ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundarySEC", 
                                                  "../../../etc/passwd", 
                                                  "malicious content");
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Path injection: file created in upload dir", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            printTestResult("Path injection: filename sanitized", 
                          filename.find("..") == std::string::npos &&
                          filename.find("/") == std::string::npos);
            printTestResult("Path injection: contains 'passwd'", 
                          filename.find("passwd") != std::string::npos);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testFilenameWithSpecialChars() {
        std::cout << BLUE << "\n=== Testing Filename with Special Characters ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundarySPEC", 
                                                  "test file with spaces.txt", 
                                                  "test content");
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Special chars: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            printTestResult("Special chars: filename has timestamp", 
                          filename.find("_") != std::string::npos);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testFilenameEmptyOrMissing() {
        std::cout << BLUE << "\n=== Testing Empty/Missing Filename ===" << RESET << std::endl;

        LocationConfig location;
        
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryEMPTY");
        
        std::string body = "------WebKitFormBoundaryEMPTY\r\n"
                          "Content-Disposition: form-data; name=\"file\"; filename=\"\"\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "content here\r\n"
                          "------WebKitFormBoundaryEMPTY--\r\n";
        
        req->_body.assign(body.begin(), body.end());
        req->_contentLength = body.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Empty filename: file created with default name", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            printTestResult("Empty filename: uses 'default'", 
                          filename.find("default") != std::string::npos);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testContentWithBoundarySubstring() {
        std::cout << BLUE << "\n=== Testing Content Containing Boundary Substring ===" << RESET << std::endl;

        LocationConfig location;
        
        std::string trickContent = "This content has ----WebKitForm in the middle of text";
        
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundaryTRICK", 
                                                  "tricky.txt", 
                                                  trickContent);
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Boundary substring: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Boundary substring: content preserved", 
                          content == trickContent);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testMultilineContent() {
        std::cout << BLUE << "\n=== Testing Multiline Content ===" << RESET << std::endl;

        LocationConfig location;
        
        std::string multilineContent = "Line 1\r\nLine 2\r\nLine 3\r\n\r\nLine 5 after blank";
        
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundaryMLINE", 
                                                  "multiline.txt", 
                                                  multilineContent);
        std::string contentType = req->_header["Content-Type"];
        
        FileHandler handler(*req, location, contentType);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Multiline content: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Multiline content: preserved", content == multilineContent);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testCopyConstructorAndAssignment() {
        std::cout << BLUE << "\n=== Testing Copy Constructor and Assignment ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = createMultipartRequest("----WebKitFormBoundaryCOPY", "test.txt", "content");
        std::string contentType = req->_header["Content-Type"];
        
        {
            FileHandler handler1(*req, location, contentType);
            FileHandler handler2(handler1);
            printTestResult("Copy constructor: compiles and runs", true);
        }
        
        {
            FileHandler handler1(*req, location, contentType);
            FileHandler handler2(*req, location, contentType);
            handler2 = handler1;
            printTestResult("Assignment operator: compiles and runs", true);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    // ========== CHUNKED PARSING TESTS ==========

    void testChunkedBoundarySplit() {
        std::cout << BLUE << "\n=== Testing Chunked: Boundary Split ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryCHUNK");
        
        // Construire le body complet
        std::string fullBody = 
            "------WebKitFormBoundaryCHUNK\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"chunked.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "This is chunked content!\r\n"
            "------WebKitFormBoundaryCHUNK--\r\n";
        
        // Découper au milieu du boundary initial
        size_t splitPos = 20; // Coupe "------WebKitFormBou" | "ndaryCHUNK\r\n..."
        std::vector<char> chunk1(fullBody.begin(), fullBody.begin() + splitPos);
        std::vector<char> chunk2(fullBody.begin() + splitPos, fullBody.end());
        
        req->_body = chunk1; // Premier chunk
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        
        // Envoyer le deuxième chunk
        handler.multiparse(chunk2);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked boundary split: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked boundary split: correct content", 
                          content == "This is chunked content!");
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedHeaderSplit() {
        std::cout << BLUE << "\n=== Testing Chunked: Header Split ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryHDR");
        
        std::string fullBody = 
            "------WebKitFormBoundaryHDR\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"header_split.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Header split content\r\n"
            "------WebKitFormBoundaryHDR--\r\n";
        
        // Découper au milieu du header Content-Disposition
        size_t splitPos = 60; // Coupe dans "Content-Dispo" | "sition: form..."
        std::vector<char> chunk1(fullBody.begin(), fullBody.begin() + splitPos);
        std::vector<char> chunk2(fullBody.begin() + splitPos, fullBody.end());
        
        req->_body = chunk1;
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        handler.multiparse(chunk2);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked header split: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked header split: correct content", 
                          content == "Header split content");
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedDataSplit() {
        std::cout << BLUE << "\n=== Testing Chunked: Data Split ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryDATA");
        
        std::string dataContent = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        
        std::stringstream bodyBuilder;
        bodyBuilder << "------WebKitFormBoundaryDATA\r\n"
                   << "Content-Disposition: form-data; name=\"file\"; filename=\"data_split.txt\"\r\n"
                   << "Content-Type: application/octet-stream\r\n"
                   << "\r\n"
                   << dataContent << "\r\n"
                   << "------WebKitFormBoundaryDATA--\r\n";
        
        std::string fullBody = bodyBuilder.str();
        
        // Découper au milieu des données
        size_t headerEnd = fullBody.find("\r\n\r\n") + 4;
        size_t splitPos = headerEnd + 18; // Coupe "ABCDEFGHIJKLMNOPQR" | "STUVWXYZ..."
        
        std::vector<char> chunk1(fullBody.begin(), fullBody.begin() + splitPos);
        std::vector<char> chunk2(fullBody.begin() + splitPos, fullBody.end());
        
        req->_body = chunk1;
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        handler.multiparse(chunk2);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked data split: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked data split: content complete", content == dataContent);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedEndBoundarySplit() {
        std::cout << BLUE << "\n=== Testing Chunked: End Boundary Split ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryEND");
        
        std::string fullBody = 
            "------WebKitFormBoundaryEND\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"end_split.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "End boundary test\r\n"
            "------WebKitFormBoundaryEND--\r\n";
        
        // Découper au milieu du boundary final
        size_t endBoundaryPos = fullBody.rfind("------WebKitFormBoundaryEND--");
        size_t splitPos = endBoundaryPos + 15; // Coupe "------WebKitFor" | "mBoundaryEND--"
        
        std::vector<char> chunk1(fullBody.begin(), fullBody.begin() + splitPos);
        std::vector<char> chunk2(fullBody.begin() + splitPos, fullBody.end());
        
        req->_body = chunk1;
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        handler.multiparse(chunk2);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked end boundary split: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked end boundary split: correct content", 
                          content == "End boundary test");
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedCRLFSplit() {
        std::cout << BLUE << "\n=== Testing Chunked: CRLF Split ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryCRLF");
        
        std::string fullBody = 
            "------WebKitFormBoundaryCRLF\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"crlf_split.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "CRLF test content\r\n"
            "------WebKitFormBoundaryCRLF--\r\n";
        
        // Découper entre \r et \n du premier \r\n
        size_t firstCRLF = fullBody.find("\r\n");
        size_t splitPos = firstCRLF + 1; // Coupe après le \r, avant le \n
        
        std::vector<char> chunk1(fullBody.begin(), fullBody.begin() + splitPos);
        std::vector<char> chunk2(fullBody.begin() + splitPos, fullBody.end());
        
        req->_body = chunk1;
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        handler.multiparse(chunk2);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked CRLF split: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked CRLF split: correct content", 
                          content == "CRLF test content");
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedMultipleSmallChunks() {
        std::cout << BLUE << "\n=== Testing Chunked: Multiple Small Chunks ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryTINY");
        
        std::string fullBody = 
            "------WebKitFormBoundaryTINY\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"tiny_chunks.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Content sent in tiny pieces!\r\n"
            "------WebKitFormBoundaryTINY--\r\n";
        
        // Découper en chunks de 10 bytes
        req->_body.clear();
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        
        // Envoyer par petits morceaux
        for (size_t i = 0; i < fullBody.size(); i += 10) {
            size_t chunkSize = std::min((size_t)10, fullBody.size() - i);
            std::vector<char> chunk(fullBody.begin() + i, fullBody.begin() + i + chunkSize);
            handler.multiparse(chunk);
        }
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked tiny pieces: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked tiny pieces: correct content", 
                          content == "Content sent in tiny pieces!");
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedByteByByte() {
        std::cout << BLUE << "\n=== Testing Chunked: Byte by Byte ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----BYTE");
        
        std::string fullBody = 
            "------BYTE\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"byte.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "ABC\r\n"
            "------BYTE--\r\n";
        
        req->_body.clear();
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        
        // Envoyer byte par byte
        for (size_t i = 0; i < fullBody.size(); i++) {
            std::vector<char> chunk(1, fullBody[i]);
            handler.multiparse(chunk);
        }
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked byte by byte: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked byte by byte: correct content", content == "ABC");
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedMultipleFilesChunked() {
        std::cout << BLUE << "\n=== Testing Chunked: Multiple Files in Chunks ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----MULTI");
        
        std::stringstream bodyBuilder;
        bodyBuilder << "------MULTI\r\n"
                   << "Content-Disposition: form-data; name=\"file1\"; filename=\"chunk1.txt\"\r\n"
                   << "Content-Type: text/plain\r\n"
                   << "\r\n"
                   << "First file content\r\n"
                   << "------MULTI\r\n"
                   << "Content-Disposition: form-data; name=\"file2\"; filename=\"chunk2.txt\"\r\n"
                   << "Content-Type: text/plain\r\n"
                   << "\r\n"
                   << "Second file content\r\n"
                   << "------MULTI--\r\n";
        
        std::string fullBody = bodyBuilder.str();
        
        // Découper entre les deux fichiers
        size_t splitPos = fullBody.find("Second") - 30; // Au milieu du boundary entre les fichiers
        
        std::vector<char> chunk1(fullBody.begin(), fullBody.begin() + splitPos);
        std::vector<char> chunk2(fullBody.begin() + splitPos, fullBody.end());
        
        req->_body = chunk1;
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        handler.multiparse(chunk2);
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked multiple files: 2 files created", fileCount == 2);
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void testChunkedLargeFileInChunks() {
        std::cout << BLUE << "\n=== Testing Chunked: Large File in Chunks ===" << RESET << std::endl;

        LocationConfig location;
        HttpRequest* req = new HttpRequest();
        req->_method = "POST";
        req->_uri = "/upload";
        req->_header.add("Content-Type", "multipart/form-data; boundary=----LARGE");
        
        // Créer un gros contenu (5KB)
        std::string largeContent;
        for (int i = 0; i < 512; i++) {
            largeContent += "0123456789";
        }
        
        std::stringstream bodyBuilder;
        bodyBuilder << "------LARGE\r\n"
                   << "Content-Disposition: form-data; name=\"file\"; filename=\"large_chunked.bin\"\r\n"
                   << "Content-Type: application/octet-stream\r\n"
                   << "\r\n"
                   << largeContent << "\r\n"
                   << "------LARGE--\r\n";
        
        std::string fullBody = bodyBuilder.str();
        
        req->_body.clear();
        req->_contentLength = fullBody.size();
        
        std::string contentType = req->_header["Content-Type"];
        FileHandler handler(*req, location, contentType);
        
        // Envoyer en chunks de 512 bytes
        for (size_t i = 0; i < fullBody.size(); i += 512) {
            size_t chunkSize = std::min((size_t)512, fullBody.size() - i);
            std::vector<char> chunk(fullBody.begin() + i, fullBody.begin() + i + chunkSize);
            handler.multiparse(chunk);
        }
        
        int fileCount = countFilesInDirectory(uploadDir);
        printTestResult("Chunked large file: file created", fileCount == 1);
        
        if (fileCount > 0) {
            std::string filename = getFirstFileInDirectory(uploadDir);
            std::string filepath = uploadDir + "/" + filename;
            std::string content = readFile(filepath);
            printTestResult("Chunked large file: correct size", content.size() == largeContent.size());
            printTestResult("Chunked large file: content matches", content == largeContent);
        }
        
        delete req;
        removeTestDirectory(uploadDir);
        createTestDirectory(uploadDir);
    }

    void runAllTests() {
        std::cout << YELLOW << "\n╔════════════════════════════════════════╗" << RESET << std::endl;
        std::cout << YELLOW << "║    FileHandler Complete Test Suite    ║" << RESET << std::endl;
        std::cout << YELLOW << "╚════════════════════════════════════════╝" << RESET << std::endl;

        // Tests de base
        testBasicSingleFileUpload();
        testMultipleFilesUpload();
        testEmptyFileUpload();
        testLargeFileUpload();
        testBinaryFileUpload();
        testFilenameWithPath();
        testFilenameWithSpecialChars();
        testFilenameEmptyOrMissing();
        testContentWithBoundarySubstring();
        testMultilineContent();
        testCopyConstructorAndAssignment();
        
        // Tests de chunked parsing
        std::cout << YELLOW << "\n--- Chunked Parsing Tests ---" << RESET << std::endl;
        testChunkedBoundarySplit();
        testChunkedHeaderSplit();
        testChunkedDataSplit();
        testChunkedEndBoundarySplit();
        testChunkedCRLFSplit();
        testChunkedMultipleSmallChunks();
        testChunkedByteByByte();
        testChunkedMultipleFilesChunked();
        testChunkedLargeFileInChunks();

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
    FileHandlerTester tester;
    tester.runAllTests();
    return 0;
}
