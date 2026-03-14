#include "CgiContainer.hpp"
#include "Response.hpp"
#include "WebServ.hpp"
#include "ANetContainer.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>

CgiContainer::CgiContainer(int epollFd, ClientHandler& parent, int fd, int event, pid_t pid) :
	AEventHandler(),
	_pid(pid), _state(0), _index(0), _parent(parent) {
	if (fd < 0)
        throw AEventHandler::HandlerException("Invalid File Descriptor passed to CGI");
	_fd = fd;
	_epollFd = epollFd;
	Logger::record(SETUP) << "Creating CGIContainer...";
	if (addToEpoll(epollFd, event) == EPOLL_CTL_FAIL)
	{
		throw AEventHandler::HandlerException("Epoll CTL fail");
	}
	_lastAlive = time(NULL);
}

CgiContainer::~CgiContainer() 
{
	// if (_fd != -1) 
	// {
	//        epoll_ctl(_epollFd, EPOLL_CTL_DEL, _fd, NULL);
	//        close(_fd);
	//        _fd = -1;
	//    }
	// _parent.unregisterCgi(this);
}

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

	if (bytes < 0)
		return CGI_READ_KO;
	if (bytes == 0)
		return CGI_READ_END;
	_CgiResult.insert(_CgiResult.end(), buffer, buffer + bytes);
	return CGI_READ_OK;
}

Response	*CgiContainer::handle_pid()
{
	int 		status;
	int 		status_code = -1;
	pid_t 		result;
	Response	*response = NULL;

	result = waitpid(this->_pid, &status, 0);
	if (result == -1)
	{
		Logger::record(ERROR) << "111111";
		response = new Response(INTERNAL_SERVER_ERROR);
	}
	else 
	{
		if (WIFEXITED(status)) 
		{
			status_code = WEXITSTATUS(status);
			if (status_code == 0)
				response = new Response(OK, this->_CgiResult, "", true);
			else
			{
				Logger::record(ERROR) << "222222222";
				response = new Response(INTERNAL_SERVER_ERROR);
			}
		} 
		else if (WIFSIGNALED(status))
			response = new Response(INTERNAL_SERVER_ERROR);
	}
	Logger::record(INFO) << "CGI status: " << status_code;
	return response;
}

int	CgiContainer::handleEvent(uint32_t event, WebServ& context) {
	bool force_end = false;

	_lastAlive = time(NULL);
	context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);
	if (event & EPOLLOUT) {
		Logger::record(INFO) << "Writing to CGI...";
		switch (handleWrite()) {
			case CGI_WRITE_KO: return CGI_KO;
			case CGI_WRITE_OK: break;
			case CGI_WRITE_END:
				Logger::record(INFO) << "Finished writing to CGI..";
				// epoll_ctl(context.getEpoll(), EPOLL_CTL_DEL, _fd, NULL);
				// close(_fd);
				// _fd = -1;
				return CGI_END;
			default: break;
		}
	}
	if (event & EPOLLIN) {
		Logger::record(INFO) << "Reading from CGI...";
		switch (handleRead()) {
			case CGI_READ_KO: return CGI_KO;
			case CGI_READ_OK: break;
			case CGI_READ_END: force_end = true;
		}
	}
	if (!force_end && (event & (EPOLLHUP | EPOLLERR))) {
        Logger::record(INFO) << "Forcing CGI response building...";
        force_end = true;
    }
	if (force_end) {
		Logger::record(INFO) << "Building CGI response";
		Response *response = this->handle_pid();
		_parent.setResponse(response);
		_parent.activateEpoll(context.getEpoll(), EPOLLOUT);
		return CGI_END;
	}
	return CGI_OK;
}

int	CgiContainer::get_type() const 
{
	return (CGI);
}
