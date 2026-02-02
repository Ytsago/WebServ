#pragma once

#include <iostream>
#include <vector>

class ANetContainer {
	public:
		ANetContainer() : socket(-1), logs(std::cout), errorLogs(std::cerr) {};
		virtual ~ANetContainer() {};
		virtual bool is_client() = 0;
		std::vector<unsigned char> msg;
		int socket;
		std::ostream&	logs;
		std::ostream&	errorLogs;
};

class Client : public ANetContainer {
	public:
		Client() : ANetContainer(), index(0) {};
		virtual ~Client();
		bool is_client() {return (true);};
		size_t	index;
};

class Server : public ANetContainer {
	public:
		Server() : ANetContainer() {};
		virtual ~Server() 	{};
		bool is_client() {return (false);};
};
