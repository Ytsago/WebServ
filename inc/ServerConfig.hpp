#pragma once

#include "LocationConfig.hpp"

class ServerConfig
{
	private:

		int							_socket;
		int							_listenPort;
		std::string					_host;
		std::string					_serverName;
		std::string					_root;
		std::string					_index;
		std::string					_errorPage;
		size_t						_clientMaxBodySize;
		LocationConfig				_defaultLocation;
		bool						_isDefaultSet;
		std::vector<LocationConfig>	_locations;

	public:

		ServerConfig();
		ServerConfig(const ServerConfig &rhs);
		ServerConfig	&operator=(const ServerConfig &rhs);
		~ServerConfig();

		int							get_socket() const;
		int							get_listen_port() const;
		std::string					get_host() const;
		std::string					get_server_name() const;
		std::string					get_root() const;
		std::string					get_index() const;
		std::string					get_error_page() const;
		size_t						get_client_max_body_size() const;
		LocationConfig				get_default_location() const;
		bool						is_default_set() const;
		std::vector<LocationConfig>	get_locations() const;

		void	set_socket(int value);
		void	set_listen_port(int value);
		void	set_host(std::string value);
		void	set_server_name(std::string value);
		void	set_root(std::string value);
		void	set_index(std::string value);
		void	set_error_page(std::string value);
		void	set_client_mbs(size_t value);
		void	set_default_location(LocationConfig value);
		void	set_is_default_set(bool value);
		void	push_location(LocationConfig value);
};

std::ostream &operator<<(std::ostream &out, const ServerConfig &serv);