#include "Client.hpp"
#include "RequestHandler.hpp"
#include <sys/socket.h>
#include <fcntl>

Client::Client(int fd, ServerConfig &server, int epollFd) : 
    this->_state(READING_REQUEST),
    _server(server),
    _epollFd(epollFd), 
    _response(NULL),
    _fileHandler(NULL),
    this->_bytesSent(0) 
{
    this->_fd = fd;
}

Client::~Client() 
{
    delete this->_response;
    delete this->_fileHandler;
    if (this->_fd != -1) close(this->_fd);
}

void Client::handleRead() 
{
    char    buffer[BUFFSIZE];
    ssize_t bytes = recv(this->_fd, buffer, BUFFSIZE, MSG_DONTWAIT);
    if (bytes <= 0) 
    {
        this->_state = END;
        return;
    }
    if (this->_state == READING_REQUEST) 
    {
        this->_parser.consume(buffer, bytes);
        if (this->_parser.headersAreComplete()) 
        {
            RequestHandler handler(this->_server, this->_parser.getRequest(), this->_epollFd);
            std::string contentType;
            if (handler.setupUpload(contentType)) 
            {
                //try catch
                this->_fileHandler = new FileHandler(this->_parser.getRequest(), handler.getLocation(), contentType);
                this->_state = WRITING_BODY;
                if (!this->_parser.getBody().empty())
                    this->_fileHandler->multiparse(this->_parser.getBody());
            } 
            else
                build_response();
        }
    } 
    else if (this->_state == WRITING_BODY) 
    {
        std::vector<char> chunk(buffer, buffer + bytes);
        this->_fileHandler->multiparse(chunk);
        if (this->_fileHandler->getState() == FileHandler::END)
            build_response();
    }
}

void Client::build_response() 
{
    RequestHandler handler(this->_server, this->_parser.getRequest(), _epollFd);
    this->_response = handler.handle_request();
    this->_state = SENDING_RESPONSE;
    //switch to epollout
}

void Client::handleWrite() 
{
    if (this->_state != SENDING_RESPONSE || !this->_response) 
        return;
    std::string &resStr = this->_response->get_full_response();
    ssize_t sent = send(this->_fd, resStr.c_str() + this->_bytesSent, resStr.size() - this->_bytesSent, 0);
    if (sent > 0) 
    {
        this->_bytesSent += sent;
        if (this->_bytesSent >= resStr.size()) 
            this->_state = END;
    }
    else if (sent == -1)
        this->_state = END;
}