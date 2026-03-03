#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <vector>

#include "ServerConfig.hpp"
#include "ConfigParser.hpp"

#define BUFFSIZE 4096

class WebServ {
	public:
		// WebServ();										//Default constructor
		WebServ(std::ostream& logStream, std::ostream& errorStream, ConfigParser &parser);
		~WebServ();										//Destructor
		WebServ(const WebServ &other);				//Copy constructor
		WebServ &operator=(const WebServ &other);	//Copy operator

		bool	run(const char *arg);
	private:
		bool	checkConnection() const;
		void	initServers(std::vector<ServerConfig>& servers);
		bool	newConnection(struct epoll_event& ev, int serverFd) const ;
		void	serverSetup(ServerConfig &server);
		void	epoll_init(std::vector<ServerConfig> &server);
		void	server_loop();

		int	_epollFd;

		static const size_t	MAXEVENT = 10;
		static const long	TIMEOUT = -1;

		std::ostream& logs;
		std::ostream& errorLogs;
		ConfigParser	&global_conf;

		static const char*	errInit;
};


#endif
