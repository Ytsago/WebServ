#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <map>
#include <list>
#include "ConfigParser.hpp"

#define BUFFSIZE 4096
#define MAXEVENT 1024
#define TIMEOUT 100

class AEventHandler;

class WebServ {
	private:
		WebServ(const WebServ& other);
		WebServ& operator=(const WebServ& other);
		
		void	initHost();
		int		setConfig(const char* arg);
		void	checkTimeout();

		std::list<AEventHandler*>		timeout;
		std::map<int, AEventHandler*>	registery;
		std::vector<ServerConfig>		serversConfig;
		int	epollFd;
	public:
		WebServ();
		~WebServ();

		void	run(const char *arg);
		void	removeHandler(AEventHandler* handler);

		int	getEpoll() const;	
		std::list<AEventHandler*>&	getTimeList();
		std::map<int, AEventHandler*>&	getRegistery();
};

#endif
