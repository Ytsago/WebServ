#ifndef SERVERHANDLER_HPP
#define SERVERHANDLER_HPP

#include "AEventHandler.hpp"
#include "ServerConfig.hpp"

class ServerHandler : public AEventHandler {
	private:
		std::vector<ServerConfig>	_config;
		int	_port;

	public:
		ServerHandler(WebServ *context, int port);

		int	handleEvent(uint32_t event, WebServ& context);
		
		const std::vector<ServerConfig>&	getConfig() const;
		int	getPort() const;
};

#endif
