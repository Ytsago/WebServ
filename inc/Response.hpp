#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <cmath>

#include "AMessage.hpp"
#include "Request.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "utils.hpp"

class Response
{
	private:

		byteVector	_full_response;
		size_t		cursor;

	public:

		Response(int code, byteVector body = byteVector(), std::string path = "", bool connection = true);
		~Response();
		Response(const Response &other);
		Response &operator=(const Response &other);

		byteVector	&get_full_response();
	
		void	build_entry_line(int code, std::string status);
		void	build_header(size_t body_size, std::string path, bool connection);
};

#endif
