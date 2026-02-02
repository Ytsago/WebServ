#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

std::vector<char>	GetFile(std::string path);
std::string			int_to_string(int n);
void	strLower(std::string& str);
bool	canBeLowered(const std::string& key);

#endif
