#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ANetContainer.hpp"
# include "HttpParser.hpp"
# include "Response.hpp"
# include "FileHandler.hpp"
# include "ServerConfig.hpp"

# define BUFFSIZE 4096

class Client : public ANetContainer 
{
    public:

        enum ClientState 
        {
            READING_REQUEST,
            WRITING_BODY,
            PROCESSING,
            SENDING_RESPONSE,
            END
        };

        Client(int fd, ServerConfig &server, int epollFd);
        ~Client();

        void    handleRead();
        void    build_response();
        void    handleWrite();
        ClientState getState() const;

    private:
    
        ClientState     _state;
        ServerConfig    &_server;
        int             _epollFd;
        HttpParser      _parser;
        Response        *_response;
        FileHandler     *_fileHandler;
        size_t          _bytesSent;
};
#endif