#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include <iostream>
# include <vector>
# include <string>
# include "Response.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"
# include "LocationConfig.hpp"

class RequestHandler
{
	private:

		const ServerConfig	&_server;
		HttpRequest			&_request;
		LocationConfig		_location;
		int					_epollFd;

	public:
	
		RequestHandler(const ServerConfig &server, HttpRequest &request, int epollFd);
		~RequestHandler();
		RequestHandler(const RequestHandler &other);
	
		Response		*handle_request();
		Response		*build_get_response(std::string &path);
		Response		*build_delete_response();

		bool			setupUpload(std::string &content_type);
		bool			get_upload_type(std::string &content_type);
		bool			get_cgi_ext(std::string &ext);
		std::string		get_file_path();
		std::string		get_cgi_path();
		void			find_corresponding_location();
		int				get_cgi_loc(std::string &ext);
		bool			is_redirection();
		byteVector		get_autodindex(const std::string& path);
		LocationConfig&	getLocation();
		const ServerConfig	&getServer();
};

#endif
