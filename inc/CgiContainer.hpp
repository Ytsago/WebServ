#ifndef CGICONTAINER_HPP
# define CGICONTAINER_HPP

# include "AEventHandler.hpp"
# include "ClientHandler.hpp"

class CgiContainer : public AEventHandler 
{
	private:
		int				_epollFd;
		pid_t			_pid;
		int				_state;
		size_t			_index;
		ClientHandler&	_parent;
		byteVector		 _CgiResult;
		const ServerConfig	*_server;

	public:
		CgiContainer(int epollFd, ClientHandler& parent, int fd, int event, pid_t pid, const ServerConfig *server);
		~CgiContainer();
		CgiContainer(const CgiContainer &other);
		CgiContainer &operator=(const CgiContainer &other);

		int			handleEvent(uint32_t event, WebServ &context);
		int			handleWrite();
		int			handleRead();
		int			get_type() const;
		Response	*handle_pid();

		bool			_isParentAlive;
};

#endif
