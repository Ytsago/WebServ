#include "WebServ.hpp"
#include <fstream>

//Experience for reading a binary file
void	fileReader() {
	std::filebuf file;

	void* pt = file.open("/home/secros/Documents/retro-mfa/MFA/blue.mfa", std::ios_base::binary | std::ios_base::in);
	if (pt == NULL) {
		std::cerr << "ERROR" << std::endl;
	}

	size_t baseSize = file.in_avail();
	std::cout << "Copy inside vector, size : " << baseSize << std::endl;
	std::vector<unsigned char>	data(baseSize);
	size_t value = file.sgetn(reinterpret_cast<char *>(data.data()), file.in_avail());
	if (value < baseSize)
		std::cerr << "PARTIAL READING /!\\" << std::endl;
	file.close();
	std::cout << "Writing output" << std::endl;
	file.open("./new.png", std::ios_base::binary | std::ios_base::out);

	file.sputn(reinterpret_cast<char*>(data.data()), data.size());
}

int main(int ac, char* const av[]) {
	(void) ac, (void)av;
	std::ofstream	logs("logs.txt");
	WebServ server(std::cout, std::cerr);

	server.run();
	// fileReader();
#include "ConfigParser.hpp"

int main(int ac, char* const av[]) 
{
	if (ac == 2)
	{
		try
		{
			ConfigParser	parser(av[1]);
			std::cout << parser;
		}
		catch (const std::exception& e) 
		{
			std::cout << e.what() << std::endl;
			return (1);
    	}
	}
}

// int main2(int ac, char* const av[]) {
// 	std::ostream& out = std::cout;
// 	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

// 	(void) ac, (void) av;
// 	sockaddr_in servAddr = {0};
// 	servAddr.sin_family = AF_INET;
// 	servAddr.sin_port = htons(PORT);
// 	servAddr.sin_addr.s_addr = INADDR_ANY;

// 	while (bind(serverSocket, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
// 		out << "ERROR, can't open socket";
// 		sleep(3);
// 	}
// 	listen(serverSocket, SOMAXCONN);

// 	int epoll_fd = epoll_create(1);
// 	struct epoll_event	baseEv, events[MAX_EVENTS];
// 	baseEv.events = EPOLLIN;
// 	baseEv.data.fd = serverSocket;
// 	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &baseEv);

// 	while (true) {
// 		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
// 		for (int i = 0; i < nfds; i++) {
// 			if (events[i].data.fd == serverSocket) {
// 				//Add a new client to the epoll list
// 				struct sockaddr_in	clientAddr;
// 				socklen_t dummyLen = sizeof(clientAddr);
// 				int clientFd = accept(serverSocket, (struct sockaddr*) &clientAddr, &dummyLen);
				
// 				Client *newClient = new Client;

// 				newClient->fd = clientFd;
// 				newClient->msg = "";
// 				baseEv.events = EPOLLIN | EPOLLET;
// 				baseEv.data.ptr =  newClient;
// 				out << "Connection received by " << clientFd << std::endl;
// 				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientFd, &baseEv);
// 			}
// 			else {
// 				if (events[i].events & EPOLLIN) {
// 					Client *client  = reinterpret_cast<Client*>(events[i].data.ptr);
// 					char	buf[BUFFERSIZE];
// 					ssize_t	bytes = 0;
// 					out << "Test" << std::endl;
// 					bytes = read(client->fd, buf, BUFFERSIZE);
// 					std::string	line(buf, bytes);
// 					if (bytes <= 0 || (line.size() == 1 && line[0] == '\n')) {
// 						if (!client->msg.empty())
// 							out << "Message: \n" << client->msg << std::endl;
// 						send(client->fd, "Pong", 4, MSG_EOR);
// 						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, 0);
// 						delete (client);
// 					}
// 					else
// 						client->msg.append(line);
// 				}
// 			}
// 		}
// 	}
// 	close(serverSocket);
// 	return 0;
// }
