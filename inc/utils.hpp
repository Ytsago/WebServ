#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

void	strLower(std::string& str);
bool	canBeLowered(const std::string& key);
void	trim(std::string& str);
std::string	extractAttribute(const std::string& header, const std::string& attName);


#endif
