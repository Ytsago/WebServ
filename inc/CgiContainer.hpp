#ifndef CGICONTAINER_HPP
# define CGICONTAINER_HPP

# include "AEventHandler.hpp"
# include "ClientHandler.hpp"

class CgiContainer : public AEventHandler 
{
	private:
		int	_outFd;
		int	_state;
		size_t	_index;
		ClientHandler&	_parent;
		byteVector	_CgiResult;

	public:
		CgiContainer(int epollFd, ClientHandler& parent, int fd, int event);
		~CgiContainer();
		CgiContainer(const CgiContainer &other);
		CgiContainer &operator=(const CgiContainer &other);

		int		handleEvent(uint32_t event, WebServ &context);
		int		handleWrite();
		int		handleRead();
		int		get_type() const;
};

#endif
