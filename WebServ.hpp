#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <map>
#include <list>
#include "color.h"
#define BUFFSIZE 4096

class AEventHandler;

class WebServ {
	private:
		WebServ(const WebServ& other);
		WebServ& operator=(const WebServ& other);

		std::list<AEventHandler*>	timeout;
		std::map<int, AEventHandler*>	registery;
		int	epollFd;
	public:
		WebServ();
		~WebServ();

		void	run();
		void	removeHandler(AEventHandler* handler);

		int	getEpoll() const;	
		std::list<AEventHandler*>&	getTimeList();
		std::map<int, AEventHandler*>&	getRegistery();

		#define SETUPEPOLL FG_ORANGE"SETUP: creating an epoll fd to manage event..."FG_RESET
};

#endif
