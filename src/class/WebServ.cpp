#include "WebServ.hpp"
#include "ConfigParser.hpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <fcntl.h>
#include <signal.h>
#include "ConfigException.hpp"
#include "ANetContainer.hpp"
#include "Recipient.hpp"

static const char	statusOk[] = "HTTP/1.1 200\r\n\r\n";
static const std::string	location("/home/halnuma/Documents/cursus/WebServ/");

sig_atomic_t running = 1;

void	sigHandler(int sig) {
	if (sig == SIGINT)
		running = 0;
}

std::vector<char> GetFile(std::string path) {
	//Open file at the end ("ate")
	std::ifstream	file(path.c_str(), std::ios::binary | std::ios::ate);

	//Get file size
	size_t	size = file.tellg();

	//Return to the start
	file.seekg(std::ios::beg);

	std::vector<char>	buffer(size + 16);
	buffer.insert(buffer.begin(), statusOk, statusOk + (sizeof(statusOk) - 1));
	if (file.read(buffer.data() + 16, size)) {
		return buffer;
	}
	return std::vector<char>();
}

WebServ::WebServ() : _epollFd(-1), logs(std::cout), errorLogs(std::cerr) {
}

WebServ::WebServ(std::ostream& logStream, std::ostream& errorStream) : _epollFd(-1), logs(logStream), errorLogs(errorStream)  {

}

WebServ::WebServ(const WebServ &other) : _epollFd(other._epollFd), logs(other.logs), errorLogs(other.errorLogs) {
}

WebServ	&WebServ::operator=(const WebServ &other) {
	(void) other;
	return (*this);
}

bool	WebServ::newConnection(struct epoll_event& ev, int serverFd) const  {
	//Add a new client to the epoll list
   	struct sockaddr_in	clientAddr;
   	socklen_t dummyLen = sizeof(clientAddr);

	logs << "Accpeting new connection." << std::endl;
   	int clientFd = accept(serverFd, (struct sockaddr*) &clientAddr, &dummyLen);
   	if (clientFd == -1)
   		return 1;

   	Client *newClient = new Client;

   	newClient->socket = clientFd;
	ev.events = EPOLLIN | EPOLLET;
   	ev.data.ptr =  newClient;

   	logs << "[LOGS] New connection on " << clientFd << std::endl;
   	epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, clientFd, &ev);

   	return 0;
}

//[TODO] Move to his own class.
void Recipient::getMsg(int fd) {
	char	buffer[BUFFSIZE];
	std::vector<char>	msg;
	int		bytes;

	while ((bytes = recv(fd, buffer, BUFFSIZE, MSG_DONTWAIT)) > 0) {
		msg.insert(msg.end(), buffer, buffer + bytes);
	}
}

int	Sender::sendMsg(Client &client) {
	size_t	bytes;

	bytes = send(client.socket, client.msg.data() + client.index, client.msg.size() - client.index, MSG_DONTWAIT);
	if (bytes < 0) 
		return -1;
	client.index += bytes;
	if (client.index == client.msg.size())
		return 1;
	return 0;
}


void	WebServ::server_loop() {
	struct epoll_event	ev, events[MAXEVENT];

	while (running)
	{
		int nfds = epoll_wait(_epollFd, events, MAXEVENT, TIMEOUT);
		for (int i = 0; i < nfds; i++) {
			ANetContainer* incoming = reinterpret_cast<ANetContainer*>(events[i].data.ptr);
			if (!incoming->is_client()) {
				while (!newConnection(ev, incoming->socket));
				logs << "[LOGS] Connection added !" << std::endl;
			}
			else if (events[i].events & EPOLLIN) {

				logs << "[LOGS] Recieving a msg from client " << incoming->socket << std::endl;
				Recipient::getMsg(incoming->socket);

				logs << "[DEBUG] Message received :\n" << incoming->msg.data() << std::endl;

				ev.events = EPOLLOUT;
				ev.data.ptr = incoming;			
				epoll_ctl(_epollFd, EPOLL_CTL_MOD, incoming->socket, &ev);
			}
			else if (events[i].events & EPOLLOUT) {
				logs << "Sending a response." << std::endl;
				/**
				Compare URI of request with paths of location and send
				corresponding index
				--> Response response = build_response([ServerConfig] server, [Request] request);
				
				if (Sender::sendMsg(GetFile(response.getRaw()), incoming->socket) == 1) {
					epoll_ctl(_epollFd, EPOLL_CTL_DEL, incoming->socket, 0);
					delete(incoming);
				}
				**/
			}
		}
	}
}

void	WebServ::epoll_init(std::vector<ServerConfig> &servers) {
	struct epoll_event	ev;
	std::vector<ServerConfig>::iterator it;

	if ((this->_epollFd = epoll_create(1)) < 0) {
		errorLogs << "Error, can't create the epoll object." << std::endl;
		throw ConfigException("tg", 2);
	}

	for (it = servers.begin(); it != servers.end(); it++)
	{
		ANetContainer *server = new Server;
		server->socket = it->get_socket();
		ev.events = EPOLLIN | EPOLLET, ev.data.ptr = &server;
		if ((epoll_ctl(_epollFd, EPOLL_CTL_ADD, it->get_socket(), &ev)) < 0)
			throw ConfigException("tg", 2);
	}
}

void	WebServ::serverSetup(ServerConfig &server) {
	sockaddr_in servAddr;
	int serverFd;
	
	logs << "[SETUP] Openning socket." << std::endl;
	if ((serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
	{
		errorLogs << "Error, can't open socket retrying in 5 sec...\n";
		throw ConfigException("tg", 2);
	}
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(server.get_listen_port());
	servAddr.sin_addr.s_addr = INADDR_ANY;
	logs << "[SETUP] Binding socket." << std::endl;
	if (bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
		errorLogs << "ERROR, can't bind socket retrying in 3 sec...\n";
		throw ConfigException("tg", 2);
	}
	logs << "[SETUP] listening." << std::endl;
	if (listen(serverFd, SOMAXCONN) < 0) {
		errorLogs << "Error, failed to listen on socket." << std::endl;
		throw ConfigException("tg", 2);
	}
	server.set_socket(serverFd);
	logs << "[SETUP] success, socket is ready !" << std::endl;
}

bool	WebServ::run(const char *arg) {
	signal(SIGINT, sigHandler);
	ConfigParser	parser(arg);
	std::vector<ServerConfig> servers = parser.get_servers();
	std::vector<ServerConfig>::iterator it;

	for (it = servers.begin(); it != servers.end(); it++)
		serverSetup(*it);
	epoll_init(servers);
	server_loop();
	std::cout << "Good ending" << std::endl;
	return 0;
}

WebServ::~WebServ() {
	if (this->_epollFd > 0)
		close(this->_epollFd);
}
