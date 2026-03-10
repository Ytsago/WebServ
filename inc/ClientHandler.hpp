#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include "AEventHandler.hpp"
#include "FileHandler.hpp"
#include "HttpParser.hpp"
#include "ServerHandler.hpp"
#include "ServerConfig.hpp"
#include "Response.hpp"

class CgiContainer;

class ClientHandler : public AEventHandler {
	public:
		ClientHandler();
		~ClientHandler();
		ClientHandler(WebServ& context, ServerHandler& host);

		int	handleEvent(uint32_t event, WebServ& context);
        int	activateEpoll(int epollFd, int event);

		int		receiveMsg(WebServ& context);
        int		build_response(int epollFd);
        void    handleWrite();

        const HttpRequest&	getRequest() const;
        Response&	getResponse();
		enum ClientState 
        {
            READING_REQUEST,
            WRITING_BODY,
            PROCESSING,
            WAITING_CGI,
            SENDING_RESPONSE,
            END
        };

	private:
		const std::vector<ServerConfig>*	_hostConf;
		HttpParser		_parser;
		HttpRequest*	_request;
		ClientState     _state;
        Response        *_response;
        FileHandler     _fileHandler;
        size_t          _bytesSent;
        CgiContainer	*_cgiIn;
        CgiContainer	*_cgiOut;

};

#endif
