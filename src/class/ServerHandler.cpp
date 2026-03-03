#include "ServerHandler.hpp"
#include "Logger.hpp"
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ClientHandler.hpp"

ServerHandler::ServerHandler(WebServ& context, std::map<int, std::vector<ServerConfig> >::iterator& it) {
	int	serverFd;
	sockaddr_in	servAddr;
	_config = it->second;

	Logger::record(SETUP) << "Openning a socket...";
	if ((serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
		Logger::record(ERROR) << "Can't open socket.";
		throw AEventHandler::HandlerException("No socket avaible.");
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(it->first);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	Logger::record(SETUP) << "Binding socket.";
	if (bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
		close(serverFd);
		Logger::record(ERROR) << "ERROR, can't bind socket to port: " << it->first;
		throw AEventHandler::HandlerException("Port is not avaible.");
	}

	Logger::record(SETUP) << "listening on port: " << it->first;
	if (listen(serverFd, SOMAXCONN) < 0) {
		close(serverFd);
		Logger::record(ERROR) << "Error, failed to listen on socket.";
		throw AEventHandler::HandlerException("Can't listen on socket.");
	}

	setSocket(serverFd);
	if (addToEpoll(context, EPOLLIN) == EPOLL_CTL_FAIL)
		throw AEventHandler::HandlerException("Epoll fail");

	Logger::record(SUCCESS) << "Success, socket is ready !";
}

const std::vector<ServerConfig>&	ServerHandler::getConfig() const {return _config;}

int	ServerHandler::handleEvent(uint32_t event, WebServ& context) {
	if (event == EPOLLIN) {
		try {
			new ClientHandler(context, *this);
		}
		catch (std::exception &e) {
			Logger::err() << e.what() << std::endl;
			return SRV_FAIL_CLT;
		}
	}
	return SRV_NEW_CLT;
}

ServerHandler::~ServerHandler() {

}
