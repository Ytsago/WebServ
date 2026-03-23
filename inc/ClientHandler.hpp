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

	int	receiveMsg(WebServ& context);
        int	build_response(int epollFd);
        void    handleWrite(WebServ& context);

        const HttpRequest&	getRequest() const;
        Response&		getResponse();
        void        		setResponse(Response *response);

        void	resetClient(int epollFd);
        void	unregisterCgi(CgiContainer* ptr);
        bool	sendTimeout(WebServ& context);
        const ServerConfig*	findHostConf();

	enum ClientState 
        {
		READING_REQUEST,
		WRITING_BODY,
        	PROCESSING,
        	WAITING_CGI,
		SENDING_RESPONSE,
		TIMED_OUT,
		END
        };

	private:
		int	_pid;
		const std::vector<ServerConfig>*	_hostConf;
		const ServerConfig*	_serverConf;
		HttpParser		_parser;
		HttpRequest*	_request;
		ClientState     _state;
        Response        *_response;
        FileHandler     *_fileHandler;
        size_t          _bytesSent;
        CgiContainer	*_cgiIn;
        CgiContainer	*_cgiOut;
        bool			_keepAlive;
		std::string		_contentType;
};

#endif
