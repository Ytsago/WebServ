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
	std::map<int, std::vector<ServerConfig> >	groups;
	std::vector<ServerConfig>::iterator it = serversConfig.begin();

	for (; it != serversConfig.end(); it++) {
		int port = it->get_listen_port();
		groups[port].push_back(*it);
	}

	for (std::map<int, std::vector<ServerConfig> >::iterator ite = groups.begin(); ite != groups.end(); ite++) {
		Logger::record(SETUP) << "Creating a new host to listen on port: " << ite->first;
		AEventHandler*	server;
		try {
			server = new ServerHandler(*this, ite);
		}
		catch (AEventHandler::HandlerException &e) {
			Logger::record(ERROR) << e.what();
			continue;
		}
		registery[server->getSocket()] = server;
	}
	if (registery.empty())
		throw std::runtime_error("No server is listening.\n Closing.");
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
		return CONFIG_KO;
	}
	Logger::record(SUCCESS) << "Config file loaded !";
	serversConfig = parser.get_servers();
	return CONFIG_OK;
}

void	WebServ::checkTimeout() {
	std::list<AEventHandler*>::reverse_iterator rit = timeout.rbegin();

	for (; rit != timeout.rend(); rit++) {
		AEventHandler*	curr = *rit;
		if (std::time(NULL) - curr->getTimeout() > TIMEOUT)
			removeHandler(curr);
		else
			return ;
	}
}

void	WebServ::run(const char *arg) {
	epoll_event	events[MAXEVENT];

	if (setConfig(arg) == CONFIG_KO)
		throw std::runtime_error("Temporary error may need to change it");
	initHost();

	while(g_running) {
		int nfds = epoll_wait(epollFd, events, MAXEVENT, TIMEOUT);
		for (int i = 0; i < nfds; i++) {
			AEventHandler* incoming = reinterpret_cast<AEventHandler*>(events[i].data.ptr);
			switch (incoming->handleEvent(events[i].events, *this)) {
				case CLT_MSG_ERR:
					removeHandler(incoming);
					break;
				case RM_CLT:
					removeHandler(incoming);
					break;
				default:
					break ;
			}
		}
		checkTimeout();
	}
}

void WebServ::removeHandler(AEventHandler* handler) {
	Logger::record(INFO) << "Removing handler: " << handler->getSocket();
	if (!handler) return;
	epoll_ctl(epollFd, EPOLL_CTL_DEL, handler->getSocket(), NULL);
	timeout.erase(handler->getTimeoutIt());
	registery.erase(handler->getSocket());
	delete handler;
}

WebServ::~WebServ() {
	std::map<int, AEventHandler*>::iterator it = registery.begin();

	for (; it != registery.end(); it++)
		delete it->second;
	if (epollFd > 0)
		close(epollFd);
}

int	WebServ::getEpoll() const {return epollFd;}
std::list<AEventHandler*>&		WebServ::getTimeList() {return timeout;}
std::map<int, AEventHandler*>&	WebServ::getRegistery() {return registery;}
