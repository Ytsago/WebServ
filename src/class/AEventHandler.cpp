#include "AEventHandler.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <unistd.h>

AEventHandler::AEventHandler(): _fd(-1) {}

int	AEventHandler::addToEpoll(int epollFd, int event) {
	epoll_event	ev;

	Logger::record(SETUP) << "Adding " << _fd << "to epoll";
	ev.events = event, ev.data.ptr = this;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _fd, &ev) < 0) {
		Logger::record(ERROR) << "Failed to add event to epoll: " << _fd;
		close(_fd);
		_fd = -1;
		return EPOLL_CTL_FAIL;
	}
	Logger::record(SUCCESS) << _fd << " added successfully";
	return EPOLL_CTL_OK;
}

AEventHandler::HandlerException::HandlerException(const char *str) : msg(str) {}
const char*	AEventHandler::HandlerException::what() const throw() {return msg;}

int	AEventHandler::getSocket() const {return _fd;}
std::list<AEventHandler*>::iterator	AEventHandler::getTimeoutIt() {return timeout_it;}
const time_t&	AEventHandler::getTimeout() const {return _lastAlive;}

void	AEventHandler::setSocket(int fd) {_fd = fd;}
void	AEventHandler::setTimeoutIt(std::list<AEventHandler*>::iterator it) {timeout_it = it;}

AEventHandler::~AEventHandler() {
	if (_fd != -1) close(_fd);
}

