#ifndef AEVENTHANDLER_HPP
#define AEVENTHANDLER_HPP

#include <ctime>
#include <list>
#include <stdint.h>
#include "WebServ.hpp"

enum	e_retState {
	SRVNEWCLT,
	SRVFAILCLT,
	CLTMSGRCV,
	CLTMSGEND,
	CLTMSGERR,
	RMCLT,
};

class AEventHandler {
	protected:
		int		_fd;
		time_t	_lastAlive;
		std::list<AEventHandler*>::iterator	timeout_it;
	public:
		AEventHandler();
		virtual ~AEventHandler();
		virtual int	handleEvent(uint32_t event, WebServ& context) = 0;

		int	getSocket() const;
		std::list<AEventHandler*>::iterator	getTimeoutIt();	
};

#endif
