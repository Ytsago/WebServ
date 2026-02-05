#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <cmath>

#include "AMessage.hpp"
#include "Request.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "utils.hpp"

class Response : public AMessage
{
	private:

	public:

		Response(int code, size_t body_size = 0, std::string path = "", bool connection = true);
		~Response();
		Response(const Response &other);
		Response &operator=(const Response &other);
	
		void	build_entry_line(int code, std::string status);
		void	build_header(size_t body_size, std::string path, bool connection);
};

#endif
