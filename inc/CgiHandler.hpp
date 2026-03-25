#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "ServerConfig.hpp"
# include "HttpRequest.hpp"
# include "Response.hpp"
#include "WebServ.hpp"
#include <stdint.h>

typedef struct s_pipe {
	int	inFd;
	int	outFd;
} t_pipe;

enum pidValue {F_NOT_FOUND = -3, F_FORBIDDEN = -2, PIPE_ERR = -1};

class CgiHandler
{
	private:

	public:

		CgiHandler();
		~CgiHandler();
		CgiHandler(const CgiHandler &other);
		CgiHandler &operator=(const CgiHandler &other);

		static t_pipe	execute_cgi(WebServ& context, const ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path, pid_t &pid);
		int	handleEvent(uint32_t event, WebServ& context);

};
#endif
