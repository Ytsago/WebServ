#include "WebServ.hpp"
#include "AEventHandler.hpp"
#include "ServerHandler.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <sys/signal.h>

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

void	WebServ::initHost() {
	std::vector<ServerConfig>::iterator it = serversConfig.begin();
	std::list<ServerHandler *> hosts;
	std::list<ServerHandler *>::iterator ite;

	for (; it != serversConfig.end(); it++) {
		for (ite = hosts.begin(); ite != hosts.end(); ite++) {
			if (ite.)
		}
	}
}

int	WebServ::setConfig(const char* arg) {
	ConfigParser	parser;

	Logger::record(SETUP) << "Reading config file: " << arg;
	try {
		parser.parse_file(arg);
	}
	catch (ConfigException &e) {
		Logger::record(ERROR) << "Failed to load configuration file.\n"
			<< e.what();
		return CONFIGKO;
	}
	Logger::record(SUCCESS) << "Config file loaded !";
	serversConfig = parser.get_servers();
	return CONFIGOK;
}

void	WebServ::run(const char *arg) {
	if (setConfig(arg) == CONFIGKO)
		throw std::runtime_error("Temporary error may need to change it");


}

void WebServ::removeHandler(AEventHandler* handler) {
	if (!handler) return;
	epoll_ctl(epollFd, EPOLL_CTL_DEL, handler->getSocket(), NULL);
	timeout.erase(handler->getTimeoutIt());
	registery.erase(handler->getSocket());
	delete handler;
}

WebServ::~WebServ() {

}

int	WebServ::getEpoll() const {return epollFd;}
std::list<AEventHandler*>&		WebServ::getTimeList() {return timeout;}
std::map<int, AEventHandler*>&	WebServ::getRegistery() {return registery;}
