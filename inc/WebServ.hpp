#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <vector>

#define BUFFSIZE 100

class WebServ {
	public:
		WebServ();										//Default constructor
		WebServ(std::ostream& logStream, std::ostream& errorStream);
		~WebServ();										//Destructor
		WebServ(const WebServ &other);				//Copy constructor
		WebServ &operator=(const WebServ &other);	//Copy operator

		bool	run();
	private:
		bool	checkConnection() const;
		bool	newConnection(int epollFd, struct epoll_event& ev) const ;
		bool	serverSetup();

		int	serverFd;

		static const size_t	PORT = 8080;
		static const size_t	MAXEVENT = 10;
		static const long	TIMEOUT = -1;

		std::ostream& logs;
		std::ostream& errorLogs;
};

class Client {
	public:
		Client() : fd(0), flags(0), logs(std::cout), errorLogs(std::cerr) {};
		~Client() 	{};
		bool methode();
		std::vector<char> msg;
		int fd;
		char flags;
		std::ostream&	logs;
		std::ostream&	errorLogs;
};

//Recipient use to receive message from a client,
//it my be usefull to make it a part of the client class
//Beware of fractionnal msg
class Recipient {
	public:
		Recipient() {};
		~Recipient() {};
		static std::vector<char>	getMsg(int fd);
};

//Sender use to send message to a client,
//it my be usefull to make it a part of the client class
//Beware of fractionnal msg
class Sender {
	public:
		static int sendMsg(std::vector<char> msg, int fd);
};
#endif
