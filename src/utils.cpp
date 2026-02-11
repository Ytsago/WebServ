#include <string>

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
