#include "ANetContainer.hpp"
#include "WebServ.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include "HttpParser.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/epoll.h>
#include <sys/stat.h>

byteVector	GetFile(std::string path) {
	std::ifstream	file(path.c_str(), std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();

	file.seekg(std::ios::beg);
	if (size == -1 || size > MAX_FILE_SIZE)
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

	if (!(ss << n))
		throw HttpParser::HttpRequestParsingException(INTERNAL_SERVER_ERROR);
	str_n = ss.str();
	return (str_n);
}

const std::string	SPECIFIC_KEYS[] = {"transfer-encoding", "connection"};

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

bool	checkIsDir(const std::string& path, bool& isDir) {
	struct stat	info;
	std::string	fullPath = path;
	if (stat(fullPath.c_str(), &info) < 0) {
		return true;
	}
	if (S_ISDIR(info.st_mode)) isDir = true;
	return false;
}

std::string	generateAutoIndex(const std::string& root, const std::string& uri) {
	DIR* dir = opendir(root.c_str());
	if (!dir)
		return "";

	std::ostringstream	html;
	html << "<html>\n<head><title>Index of " << uri << "</title></head>\n";
	html << "<body>\n<h1>Index of " << uri << "</h1><hr><pre>\n";
	struct dirent*	entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string	name = entry->d_name;
		bool		isDir = false;
		if (name == "." || name == "..")
			continue;

		switch (entry->d_type) {
			case DT_DIR: isDir = true; break;
			case DT_REG: isDir = false; break;
			case DT_LNK:
			case DT_UNKNOWN:
				if (!checkIsDir(root + "/" + entry->d_name, isDir)) {
					Logger::record(WARNING) << "Can't determinate file: " << name;
					continue;
				}
				break;
			default:
				Logger::record(WARNING) << "Unknow file entry: " << name;
				continue;
		}
		std::string	link = (*uri.rbegin() == '/') ? uri + name : uri + "/" + name;
		html << "<a href=\"" <<	link + (isDir ? "/" : "") << "\">" << name + (isDir ? "/" : "") << "</a>\n";
	}
	html << "</pre><hr></body>\n</html>";
	closedir(dir);
	return html.str();
}
