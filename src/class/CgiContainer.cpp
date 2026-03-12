#include "CgiContainer.hpp"
#include "WebServ.hpp"
#include "ANetContainer.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"
#include <sys/epoll.h>
#include <unistd.h>

CgiContainer::CgiContainer(int epollFd, ClientHandler& parent, int fd, int event) :
	AEventHandler(),
	_state(0), _index(0), _parent(parent) {
	_fd = fd;
	Logger::record(SETUP) << "Creating CGIContainer...";
	Logger::record(SETUP) << "Forking...";
	if (addToEpoll(epollFd, event) == EPOLL_CTL_FAIL)
		throw AEventHandler::HandlerException("Epoll CTL fail");
	_lastAlive = time(NULL);
}

CgiContainer::~CgiContainer() {}

int	CgiContainer::handleWrite() {
	const byteVector&	body =_parent.getRequest().getBody();
	if (body.size() == 0) {
		Logger::record(DEBUG) << "Body: " << body.size();
		return CGI_WRITE_END;
	}
	ssize_t	bytes = write(_fd, body.data() + _index, body.size() - _index);
	if (bytes < 0)
		return CGI_WRITE_KO;
	if (bytes == 0 || _index + bytes == body.size()) {
		return CGI_WRITE_END;
	}
	_index += bytes;
	Logger::record(DEBUG) << "Write handle";
	return CGI_WRITE_OK;
}

int	CgiContainer::handleRead() {
	unsigned char buffer[BUFFSIZE];
	ssize_t	bytes = read(_fd, buffer, BUFFSIZE);
	if (bytes < 0)
		return CGI_READ_KO;
	if (bytes == 0)
		return CGI_READ_END;

	_CgiResult.insert(_CgiResult.end(), buffer, buffer + bytes);
	return CGI_READ_OK;
}

int	CgiContainer::handleEvent(uint32_t event, WebServ& context) {
	if (event & EPOLLIN) {
		Logger::record(INFO) << "Reading from CGI...";
		switch (handleRead()) {
			case CGI_READ_KO: return CGI_KO;
			case CGI_READ_OK: break;
			case CGI_READ_END:
				Logger::record(INFO) << "Building CGI response";
				byteVector	response = _parent.getResponse().get_full_response();
				response.insert(response.end(), _CgiResult.begin(), _CgiResult.end());
				_parent.activateEpoll(context.getEpoll(), EPOLLOUT);
				return CGI_END;
		}
	}
	else if (event & EPOLLOUT) {
		Logger::record(INFO) << "Writing to CGI...";
		switch (handleWrite()) {
			case CGI_WRITE_KO: return CGI_KO;
			case CGI_WRITE_OK: break;
			case CGI_WRITE_END:
				Logger::record(INFO) << "Finished writing to CGI..";
				epoll_ctl(context.getEpoll(), EPOLL_CTL_DEL, _fd, NULL);
				return CGI_WRITE_END;
			default: break;
		}
	}
	return CGI_OK;
}

int	CgiContainer::get_type() const 
{
	return (CGI);
}
