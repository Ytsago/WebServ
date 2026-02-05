#include <vector>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "AMessage.hpp"
#include "ANetContainer.hpp"

byteVector	GetFile(std::string path) {
	//Open file at the end ("ate")
	std::ifstream	file(path.c_str(), std::ios::binary | std::ios::ate);

	//Get file size
	size_t	size = file.tellg();

	//Return to the start
	file.seekg(std::ios::beg);

	byteVector	buffer;
	if (file.read(buffer.data() + 16, size)) {
		return buffer;
	}
	return byteVector();
}

std::string	int_to_string(int n)
{
	std::stringstream 	ss;
	std::string			str_n;

	ss << n;
	str_n = ss.str();
	return (str_n);
}

const std::string	SPECIFIC_KEYS[] = {"transfer-encoding", "connection"}; //ADD here the key that will be lowered

void	strLower(std::string& str) {
	std::string::iterator	it = str.begin();

	for (; it != str.end(); it++)
		*it = tolower(*it);
}

bool	canBeLowered(const std::string& key) {
	for (size_t i = 0; i < sizeof(SPECIFIC_KEYS) / sizeof(std::string); i++)
		if (key == SPECIFIC_KEYS[i])
			return true;
	return false;
}

bool	add_to_epoll(int epollFd, int fd, int event, ANetContainer *container)
{
	struct epoll_event ev;
    ev.events = event;
    ev.data.ptr = container;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
		return false;
   	return true;
}