#ifndef SERVERHANDLER_HPP
#define SERVERHANDLER_HPP

#include "AEventHandler.hpp"
#include "ServerConfig.hpp"

class ServerHandler : public AEventHandler {
	private:
		std::vector<ServerConfig>	_config;
	public:
		ServerHandler(WebServ& context, std::map<int, std::vector<ServerConfig> >::iterator& it);
		~ServerHandler();

		int	handleEvent(uint32_t event, WebServ& context);
		
		const std::vector<ServerConfig>&	getConfig() const;
		int	getPort() const;
};

#endif
