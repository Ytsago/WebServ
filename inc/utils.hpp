#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

#include "ANetContainer.hpp"


std::vector<char>	GetFile(std::string path);
std::string			int_to_string(int n);
void				strLower(std::string& str);
bool				canBeLowered(const std::string& key);
bool				add_to_epoll(int epollFd, int fd, int event, ANetContainer *container);
void				trim(std::string& str);
std::string			extractAttribute(const std::string& header, const std::string& attName);
std::string			generateAutoIndex(const std::string& root, const std::string& uri);


#endif
