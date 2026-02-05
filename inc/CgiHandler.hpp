#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "ServerConfig.hpp"
# include "HttpRequest.hpp"
# include "Response.hpp"
# include "Request.hpp"

class CgiHandler
{
	private:

	public:

		CgiHandler();
		~CgiHandler();
		CgiHandler(const CgiHandler &other);
		CgiHandler &operator=(const CgiHandler &other);

		static void	execute_cgi(ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path, int &epollFd);

};
#endif