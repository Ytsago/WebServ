#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "ServerConfig.hpp"
# include "HttpRequest.hpp"
# include "Response.hpp"
# include "Request.hpp"
#include "WebServ.hpp"
#include <stdint.h>

class CgiHandler
{
	private:

	public:

		CgiHandler();
		~CgiHandler();
		CgiHandler(const CgiHandler &other);
		CgiHandler &operator=(const CgiHandler &other);

		static void	execute_cgi(ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path, int &epollFd);
		int	handleEvent(uint32_t event, WebServ& context);

};
#endif
