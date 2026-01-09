#include <cstring>
#include <fstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

# define PORT 8080
# define MAX_EVENTS 10
# define TIMEOUT -1
# define BUFFERSIZE 1024

# define METHOD 0b00000001

class Client {
	public:
		Client() : fd(0), flags(0), logs(std::cout), errorLogs(std::cerr) {};
		~Client() 	{close(fd);}
		bool methode();
		std::string msg;
		int fd;
		char flags;
		std::ostream&	logs;
		std::ostream&	errorLogs;
};

bool	Client::methode() {
	return flags & METHOD;
}

char endPattern[4] = {'\r', '\n', '\r', '\n'};
char endFieldPat[2] = {'\r', '\n'};

int	parseLine(Client* client, std::string& line) {
	if (!client->methode()) {
		std::stringstream iss(line);
		std::string methode, target, version;
		iss >> methode >> target >> version;
		client->flags |= METHOD;
		client->msg = "";
		if (methode.empty() || target.empty() || version.empty()) {
			client->errorLogs << "Error, The methode format is incorrect." << std::endl;	
			return -1;
		}
		else 
			client->logs << "Methode parse: " << methode << ", " << target << ", " << version << std::endl;
	}
	else {
		size_t	sep = line.find(':');
		std::string fieldName, fieldValue;
		fieldName = line.substr(0, sep);
		fieldValue = line.substr(sep + 1, line.size());
		if (sep == line.npos || fieldName.empty() || fieldValue.empty() || fieldValue[0] == '\n') {
			client->errorLogs << "Error, The field format is incorrect" << std::endl;
			client->errorLogs << line;
		}
		else
			client->logs << "Field parse: " << fieldName << " | " << fieldValue << std::endl;
	}
	client->msg = "";
	return 0;
}

std::string	getSimpleResponds() {
	std::ifstream	infile("/home/secros/Documents/Workshop/Web/index.html");
	std::string		output;

	while (infile) {
		infile.
	}
	return output;
}

int	getSimpleRequest(Client* client) {
	char	buf[BUFFERSIZE];
	ssize_t	bytes = 0;
	std::string	line;
	size_t		pos;

	while ((bytes = read(client->fd, buf, BUFFERSIZE)) > 0) {
		line.append(buf, bytes);
		if (line.rfind(endPattern))
			break;
	}
	return 0;
}

int main(int ac, char* const av[]) {
	std::cout << getSimpleResponds() << std::endl;
	std::ostream& out = std::cout;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	(void) ac, (void) av;
	sockaddr_in servAddr = {0};
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	while (bind(serverSocket, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
		out << "ERROR, can't open socket";
		sleep(3);
	}
	listen(serverSocket, SOMAXCONN);

	int epoll_fd = epoll_create(1);
	struct epoll_event	baseEv, events[MAX_EVENTS];
	baseEv.events = EPOLLIN;
	baseEv.data.fd = serverSocket;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &baseEv);

	while (true) {
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
		for (int i = 0; i < nfds; i++) {
			if (events[i].data.fd == serverSocket) {
				//Add a new client to the epoll list
				struct sockaddr_in	clientAddr;
				socklen_t dummyLen = sizeof(clientAddr);
				int clientFd = accept(serverSocket, (struct sockaddr*) &clientAddr, &dummyLen);
				
				Client *newClient = new Client;

				newClient->fd = clientFd;
				newClient->msg = "";
				baseEv.events = EPOLLIN | EPOLLET;
				baseEv.data.ptr =  newClient;
				out << "Connection received by " << clientFd << std::endl;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientFd, &baseEv);
			}
			else {
				Client *client  = reinterpret_cast<Client*>(events[i].data.ptr);
				if (events[i].events & EPOLLIN) {
					getSimpleRequest(client);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, 0);
					baseEv.events = EPOLLOUT;
					baseEv.data.ptr = client;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client->fd, &baseEv);
				}
				else {
					send(client->fd, getSimpleResponds().c_str(), getSimpleResponds().size(), 0);
				}
			}
		}
	}
	close(serverSocket);
	return 0;
}
