#pragma once

#include <exception>

#include "ServerConfig.hpp"
#include "ConfigException.hpp"

class LocationConfig;

class ConfigParser
{
	private:

		std::vector<ServerConfig>	_servers;
		int							_lineCount;

	public:

		ConfigParser();
		ConfigParser(const char *arg);
		ConfigParser(const ConfigParser &rhs);
		ConfigParser &operator=(const ConfigParser &rhs);
		~ConfigParser();

		std::vector<ServerConfig>	get_servers() const;

		void			parse_file(const char *arg);
		ServerConfig	parse_server(std::ifstream &file);
		LocationConfig	parse_location(std::ifstream &file, std::string header);
};

std::ostream &operator<<(std::ostream &out, const ConfigParser &conf);