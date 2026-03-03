#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include "AEventHandler.hpp"
#include "HttpParser.hpp"
#include "ServerHandler.hpp"
#include "ServerConfig.hpp"

class ClientHandler : public AEventHandler {
	private:
		const std::vector<ServerConfig>*	_hostConf;
		HttpParser		_parser;
		HttpRequest*	_request;
	public:
		ClientHandler();
		~ClientHandler();
		ClientHandler(WebServ& context, ServerHandler& host);

		int	handleEvent(uint32_t event, WebServ& context);

		int	receiveMsg();
		int	sendMsg();
};

#endif
