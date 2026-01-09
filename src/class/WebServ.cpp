#include "WebServ.hpp"

WebServ::WebServ() : logs(std::cout), errorLogs(std::cerr) {
}

WebServ::WebServ(std::ostream& logStream, std::ostream& errorStream) : logs(logStream), errorLogs(errorStream)  {

}

WebServ::WebServ(const WebServ &other) : logs(other.logs), errorLogs(other.errorLogs) {
}

WebServ	&WebServ::operator=(const WebServ &other) {
	if (this != & other) {
		this->logs = other.logs;
		this->errorLogs = other.errorLogs;
	}
	return (*this);
}

WebServ::run() {
	int	serverFd;
	sockaddr_int servAddr;

	while ((serverFd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
		WebServ.errorLogs << "Error, can't open socket retrying in 5 sec...\n";
		sleep(5);
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	while (bind(serverSocket, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
		WebServ.errorLogs<< "ERROR, can't bind socket retrying in 5 sec...\n";
		sleep(3);
	}

	WebServ.logs << "Socket is ready for listening operation !" << std::endl;

	if (listen(serverFd, SOMAXCONN) < 0) {
		WebServ.errorLogs << "Error, failed to listen on socket." << std::endl;
		return -1;
	}

}

WebServ::~WebServ() {
}
