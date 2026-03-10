#include <vector>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "AMessage.hpp"
#include "Logger.hpp"
#include "ANetContainer.hpp"

byteVector	GetFile(std::string path) {
	//Open file at the end ("ate")
	std::ifstream	file(path.c_str(), std::ios::binary | std::ios::ate);

	//Get file size
	ssize_t	size = file.tellg();


	//Return to the start
	file.seekg(std::ios::beg);

	if (size < 1)
		return byteVector();
	byteVector	buffer(size);
	if (file.read(buffer.data(), size)) {
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
void	trim(std::string& str) {
	str.erase(0, str.find_first_not_of(" \t"));
	size_t	last = str.find_last_not_of(" \t");

	if (last != std::string::npos)
		str.erase(last + 1);
}

std::string	extractAttribute(const std::string& header, const std::string& attName) {
	std::string	tmp(header);
	strLower(tmp);

	size_t	attIndex = tmp.find(attName);
	while (attIndex != std::string::npos) {
		if (attIndex == 0 || tmp[attIndex -1] == ' ' || tmp[attIndex -1] == ';')
			break;
		attIndex = tmp.find(attName, attIndex + 1);
	}
	if (attIndex == std::string::npos) return "";

	size_t len;
	size_t attEnd = tmp.find_first_of(" \t;", attIndex + attName.size());
	if (attEnd == std::string::npos) len = std::string::npos;
	else len = attEnd - attIndex - attName.size();

	std::string attribute = header.substr(attIndex + attName.size(), len);
	if (attribute.size() >= 2 && attribute[0] == '"' && attribute[attribute.size() -1] == '"')
		return attribute.substr(1, attribute.size() -2);
	return attribute;
}
