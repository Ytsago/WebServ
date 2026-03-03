#ifndef AEVENTHANDLER_HPP
#define AEVENTHANDLER_HPP

#include <ctime>
#include <list>
#include <stdint.h>
#include <exception>
#include "WebServ.hpp"

enum	e_retState {
	CONFIG_OK,
	CONFIG_KO,
	SRV_NEW_CLT,
	SRV_FAIL_CLT,
	CLT_MSG_RCV,
	CLT_MSG_END,
	CLT_MSG_ERR,
	RM_CLT,
	EPOLL_CTL_OK,
	EPOLL_CTL_FAIL,
};

class AEventHandler {
	protected:
		int		_fd;
		time_t	_lastAlive;
		std::list<AEventHandler*>::iterator	timeout_it;

		void	setSocket(int fd);
		int	addToEpoll(WebServ& context, int event);
	public:
		class HandlerException : public std::exception {
			private:
				const char* msg;
			public:
				HandlerException(const char* str);
				const char*	what() const throw();
		};

		AEventHandler();
		virtual ~AEventHandler();
		virtual int	handleEvent(uint32_t event, WebServ& context) = 0;

		int	getSocket() const;
		const time_t&	getTimeout() const;
		std::list<AEventHandler*>::iterator	getTimeoutIt();	
};

#endif
