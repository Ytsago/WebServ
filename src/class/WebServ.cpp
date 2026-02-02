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
#include "Client.hpp"
#include "Server.hpp"
#include "Recipient.hpp"
#include "utils.hpp"

static const std::string	location("./website/");

sig_atomic_t running = 1;

void	sigHandler(int sig) {
	if (sig == SIGINT)
		running = 0;
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

   	ANetContainer *newClient = new Client;

   	newClient->setSocket(clientFd);
	ev.events = EPOLLIN;
   	ev.data.ptr =  newClient;

   	logs << "[LOGS] New connection on " << clientFd << std::endl;
   	epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, clientFd, &ev);

   	return 0;
}

//[TODO] Move to his own class.
void Recipient::getMsg(Client* client) {
	char	buffer[BUFFSIZE];
	int		bytes;

	bytes = recv(client->getSocket(), buffer, BUFFSIZE, MSG_DONTWAIT);
	client->getRequest().append(buffer, bytes);
	client->getRequest().processMsg();
	// std::cout << bytes << " EOF: "<< client->getRequest().eof() << std::endl; REMOVE
}

//[TODO] Change the argument when the client handle request and respond itself
int	Sender::sendMsg(Client* client, std::vector<char> msg) {
	size_t	bytes;

	bytes = send(client->getSocket(), msg.data() + client->getIndex(), msg.size() - client->getIndex(), MSG_DONTWAIT);
	if (bytes < 0) 
		return -1;
	client->setIndex(client->getIndex() + bytes);
	if (client->getIndex() == msg.size())
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
			if (!incoming->isClient()) {
				while (!newConnection(ev, incoming->getSocket()));
				logs << "[LOGS] Connection added !" << std::endl;
			}
			else if (events[i].events & EPOLLIN) {
				Client* client = dynamic_cast<Client*>(incoming);
				logs << "[LOGS] Recieving a msg from client " << incoming->getSocket() << std::endl;
				Recipient::getMsg(dynamic_cast<Client*>(incoming));

				// logs << "[DEBUG] Message received :\n" << incoming->msg.data() << '\0' << std::endl;
				if (client->getRequest().eof()) {
					logs << "[LOGS] Message: \n" << client->getRequest() << std::endl;
					ev.events = EPOLLOUT;
					ev.data.ptr = incoming;			
					epoll_ctl(_epollFd, EPOLL_CTL_MOD, incoming->getSocket(), &ev);
				}
			}
			else if (events[i].events & EPOLLOUT) {
				Client* client = dynamic_cast<Client*>(incoming);
				logs << "Sending a response." << std::endl;
				//[TODO] Logic broken here
				if (Sender::sendMsg(dynamic_cast<Client*>(incoming), GetFile(location + client->getRequest().get_uri()))) {
					epoll_ctl(_epollFd, EPOLL_CTL_DEL, incoming->getSocket(), 0);
					delete(incoming);
				}
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
		server->setSocket(it->get_socket());
		ev.events = EPOLLIN, ev.data.ptr = server;
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
