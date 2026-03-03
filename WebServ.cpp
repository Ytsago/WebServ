#include "WebServ.hpp"
#include "AEventHandler.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <sys/signal.h>
#include "color.h"

bool	g_running = true;

void	sigHandler(int sig) {
	if (sig == SIGINT)
		g_running = false;
}

WebServ::WebServ() {

	Logger::record(SETUP) << "Creating epoll fd to manage event..."; 
	epollFd = epoll_create(1);
	if (epollFd < 0) {
		Logger::record(ERROR) << "Failed to create the epoll loop";
		throw std::runtime_error("Error, epoll failed to load...");
	}

	Logger::record(SETUP) << "Creating the singal handler...";
	signal(SIGINT, sigHandler);

	Logger::record(SUCCESS) << "You can safely quit the program with CTRL + C";
}

void	WebServ::run() {

}

void WebServ::removeHandler(AEventHandler* handler) {
	if (!handler) return;
	epoll_ctl(epollFd, EPOLL_CTL_DEL, handler->getSocket(), NULL);
	timeout.erase(handler->getTimeoutIt());
	registery.erase(handler->getSocket());
	delete handler;
}

int	WebServ::getEpoll() const {return epollFd;}
std::list<AEventHandler*>&		WebServ::getTimeList() {return timeout;}
std::map<int, AEventHandler*>&	WebServ::getRegistery() {return registery;}
