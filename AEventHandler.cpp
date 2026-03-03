#include "AEventHandler.hpp"
#include <unistd.h>

AEventHandler::AEventHandler(): _fd(-1) {}

AEventHandler::~AEventHandler() {
	if (_fd != -1) close(_fd);
}

int	AEventHandler::getSocket() const {return _fd;}
std::list<AEventHandler*>::iterator	AEventHandler::getTimeoutIt() {return timeout_it;}
