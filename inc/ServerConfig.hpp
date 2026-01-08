#pragma once

#include "LocationConfig.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>

class ServerConfig
{
	private:

		int							_listen_port;
		std::string					_host;
		std::string					_server_name;
		std::string					_root;
		std::string					_index;
		std::string					_error_page;
		size_t						_client_max_body_size;
		// std::vector<LocationConfig>	_locations;

	public:

		ServerConfig();
		~ServerConfig();

		void	set_listen_port(int value);
		void	set_host(std::string value);
		void	set_server_name(std::string value);
		void	set_root(std::string value);
		void	set_index(std::string value);
		void	set_error_page(std::string value);
		void	set_client_mbs(size_t value);

		void	print() const;
};