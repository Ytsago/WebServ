#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <vector>

class ServerConfig;

class LocationConfig
{
	private:

		std::string					_path;
		std::string					_root;
		std::string					_index;
		std::vector<std::string>	_methods;
		bool						_autoindex;
		std::string					_cgiExt;
		std::string					_cgiPath;
		bool						_isCgi;

	public:

		LocationConfig();
		LocationConfig(const LocationConfig &rhs);
		LocationConfig	&operator=(const LocationConfig &rhs);
		~LocationConfig();

		std::string					get_path() const;
		std::string					get_root() const;
		std::string					get_index() const;
		std::vector<std::string>	get_methods() const;
		bool						get_autoindex() const;
		std::string					get_cgi_ext() const;
		std::string					get_cgi_path() const;
		bool						is_cgi() const;

		void	set_path(std::string value);
		void	set_root(std::string value);
		void	set_index(std::string value);
		void	push_method(std::string value);
		void	set_autoindex(bool value);
		void	set_cgi_ext(std::string value);
		void	set_cgi_path(std::string value);
		void	set_is_cgi(bool value);
};

std::ostream &operator<<(std::ostream &out, const LocationConfig &location);