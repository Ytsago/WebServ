#include "CgiContainer.hpp"
#include "Response.hpp"
#include "WebServ.hpp"
#include "ANetContainer.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

CgiContainer::CgiContainer(int epollFd, ClientHandler& parent, int fd, int event) :
	AEventHandler(),
	_state(0), _index(0), _parent(parent) {
	_fd = fd;
	_epollFd = epollFd;
	Logger::record(SETUP) << "Creating CGIContainer...";
	if (addToEpoll(epollFd, event) == EPOLL_CTL_FAIL)
	{
		throw AEventHandler::HandlerException("Epoll CTL fail");
	}
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
	return CGI_WRITE_OK;
}

int	CgiContainer::handleRead() {
	unsigned char buffer[BUFFSIZE];
	ssize_t	bytes = read(_fd, buffer, BUFFSIZE);
	std::cout << "bytes: " << bytes;
	if (bytes < 0)
		return CGI_READ_KO;
	_CgiResult.insert(_CgiResult.end(), buffer, buffer + bytes);
	if (bytes < BUFFSIZE || bytes == 0)
		return CGI_READ_END;
	return CGI_READ_OK;
}

int	CgiContainer::handleEvent(uint32_t event, WebServ& context) {
	if (event & EPOLLIN) {
		Logger::record(INFO) << "Reading from CGI...";
		switch (handleRead()) {
			case CGI_READ_KO:
				Logger::record(INFO) << "Reading KO";
				return CGI_KO;
			case CGI_READ_OK:
				Logger::record(INFO) << "Reading OK";
				break;
			case CGI_READ_END:
				Logger::record(INFO) << "Building CGI response";
				Response *response = new Response(OK, _CgiResult, "", true);
				_parent.setResponse(response);
				// Logger::record(INFO) << "CGI result: " << response->get_full_response().data();
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
