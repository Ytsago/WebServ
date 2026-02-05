#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#include "ANetContainer.hpp"


std::vector<char>	GetFile(std::string path);
std::string			int_to_string(int n);
void				strLower(std::string& str);
bool				canBeLowered(const std::string& key);
bool				add_to_epoll(int epollFd, int fd, int event, ANetContainer *container);


#endif
