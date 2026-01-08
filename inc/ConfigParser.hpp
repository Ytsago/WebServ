#pragma once

#include "ServerConfig.hpp"

class LocationConfig;

class ConfigParser
{
	private:

		ServerConfig    _servConf;
		LocationConfig  _locConf;

	public:

		ConfigParser();
		ConfigParser(const char *arg);
		~ConfigParser();

		void	parse_input(const char *arg);
		void	parse_server(std::ifstream &file);
		void	print_conf() const;
};