#include "ServerHandler.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include "ClientHandler.hpp"

ServerHandler::ServerHandler(WebServ *context, int port) : port(port) {

}

const std::vector<ServerConfig>&	ServerHandler::getConfig() const {return _config;}

int	ServerHandler::handleEvent(uint32_t event, WebServ& context) {
	if (event == EPOLLIN) {
		try {
			new ClientHandler(context, *this);
		}
		catch (std::exception &e) {
			Logger::err() << e.what() << std::endl;
			return SRVFAILCLT;
		}
	}
	return SRVNEWCLT;
}
