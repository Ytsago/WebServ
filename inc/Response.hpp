#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <cmath>

#include "WebServ.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "utils.hpp"

#define ERROR_PAGE "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>Error</title>\n</head>\n<body>\n<h1>"
#define ERROR_PAGE_END "</h1>\n</body>\n</html>\n"

class Response
{
	private:

		byteVector	_full_response;
		int			_status_code;

	public:

		Response(int code, byteVector body = byteVector(), std::string path = "", bool connection = false);
		~Response();
		Response(const Response &other);
		Response &operator=(const Response &other);

		byteVector	&get_full_response();
		int			getStatusCode() const;
	
		void	build_entry_line(int code, std::string status);
		void	build_header(size_t body_size, std::string path, bool connection);
};

#endif
