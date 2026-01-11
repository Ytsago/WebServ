#include "WebServ.hpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <fcntl.h>

static const char	statusOk[] = "HTTP/1.1 200\r\n\r\n";
static const std::string	location("/home/secros/Documents/Workshop/Web/");

std::vector<char> GetFile(std::string path) {
	//Open file at the end ("ate")
	std::ifstream	file(path.c_str(), std::ios::binary | std::ios::ate);

	//Get file size
	size_t	size = file.tellg();

	//Return to the start
	file.seekg(std::ios::beg);

	std::vector<char>	buffer(size + 16);
	buffer.insert(buffer.begin(), statusOk, statusOk + 16);
	if (file.read(buffer.data() + 16, size)) {
		return buffer;
	}
	return std::vector<char>();
}

WebServ::WebServ() : logs(std::cout), errorLogs(std::cerr) {
}

WebServ::WebServ(std::ostream& logStream, std::ostream& errorStream) : logs(logStream), errorLogs(errorStream)  {

}

WebServ::WebServ(const WebServ &other) : logs(other.logs), errorLogs(other.errorLogs) {
}

WebServ	&WebServ::operator=(const WebServ &other) {
	(void) other;
	return (*this);
}

bool	WebServ::newConnection(int epollFd, struct epoll_event& ev) const  {
	//Add a new client to the epoll list
   	struct sockaddr_in	clientAddr;
   	socklen_t dummyLen = sizeof(clientAddr);

	logs << "Accpeting new connection." << std::endl;
   	int clientFd = accept(serverFd, (struct sockaddr*) &clientAddr, &dummyLen);
   	if (clientFd == -1)
   		return 0;
   		
   	Client *newClient = new Client;

   	newClient->fd = clientFd;
	ev.events = EPOLLIN | EPOLLET;
   	ev.data.ptr =  newClient;

   	logs << "[LOGS] New connection on " << clientFd << std::endl;
   	epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);

   	return clientFd;
}

//[TODO] Move to his own class.
std::vector<char> Recipient::getMsg(int fd) {
	char	buffer[BUFFSIZE];
	std::vector<char>	msg;
	int		bytes;

	while ((bytes = recv(fd, buffer, BUFFSIZE, MSG_DONTWAIT)) > 0) {
		msg.insert(msg.end(), buffer, buffer + bytes);
	}
	return msg;
}

int	Sender::sendMsg(std::vector<char> msg, int fd) {
	static size_t i = 0; //[TODO] Has to be personnal
	size_t	bytes;

	bytes = send(fd, msg.data() + i, msg.size() - i, MSG_DONTWAIT);
	if (bytes < 0) 
		return -1;
	i += bytes;
	if (i == msg.size())
		return 1;
	return 0;
}

bool	WebServ::checkConnection() const {
	int epollFd;
	struct epoll_event	ev, events[MAXEVENT];

	if ((epollFd = epoll_create(1)) < 0) {
		errorLogs << "Error, can't create the epoll object." << std::endl;
		return -1;
	}

	Client server;
	server.fd = serverFd;
	ev.events = EPOLLIN | EPOLLET, ev.data.ptr = &server;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev);

	logs << "[LOGS] Epoll is waiting for connection." << std::endl;

	while (1) {
		int nfds = epoll_wait(epollFd, events, MAXEVENT, TIMEOUT);
		for (int i = 0; i < nfds; i++) {
			Client* client = reinterpret_cast<Client*>(events[i].data.ptr);
			if (events[i].data.ptr == &server) {
				while(newConnection(epollFd, ev));
				logs << "[LOGS] Connection added !" << std::endl;
			}
			else if (events[i].events & EPOLLIN) {

				logs << "[LOGS] Recieving a msg from client " << client->fd << std::endl;
				client->msg = Recipient::getMsg(client->fd);

				// logs << "[DEBUG] Message received :\n" << client->msg.data() << std::endl;

				ev.events = EPOLLOUT;
				ev.data.ptr = client;			
				epoll_ctl(epollFd, EPOLL_CTL_MOD, client->fd, &ev);
			}
			else if (events[i].events & EPOLLOUT) {
				logs << "Sending a response." << std::endl;
				if (Sender::sendMsg(GetFile(location), client->fd) == 1) {
					epoll_ctl(epollFd, EPOLL_CTL_DEL, client->fd, 0);
					close(client->fd);
					delete(client);
				}
			}
	   }
	}
}

bool	WebServ::serverSetup() {
	sockaddr_in servAddr;

	logs << "[SETUP] Openning socket." << std::endl;
	while ((serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
		errorLogs << "Error, can't open socket retrying in 5 sec...\n";
		sleep(5);
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	logs << "[SETUP] binding socket." << std::endl;
	while (bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
		errorLogs<< "ERROR, can't bind socket retrying in 5 sec...\n";
		sleep(3);
	}


	logs << "[SETUP] listening." << std::endl;
	if (listen(serverFd, SOMAXCONN) < 0) {
		errorLogs << "Error, failed to listen on socket." << std::endl;
		return 1;
	}

	logs << "[SETUP] success, socket is ready !" << std::endl;
	return 0;
}

bool	WebServ::run() {
	//[TODO] Make the setup happend for all the server
	if (serverSetup()) {
		logs << "Setup failed" << std::endl;
		return 1;
	}

	//[TODO] Check to make it work for multiple server
	//[TODO] Security issue
	if (checkConnection()) {
		logs << "Connection failed" << std::endl;
		return 1;
	}
	return 0;
}

WebServ::~WebServ() {
}
