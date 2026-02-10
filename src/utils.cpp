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
