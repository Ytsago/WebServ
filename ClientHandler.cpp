#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

ClientHandler::ClientHandler(): AEventHandler(), _hostConf(NULL) {
}

ClientHandler::ClientHandler(WebServ& context, ServerHandler& host) {
	if ((_fd = accept(host.getSocket(), NULL, NULL)) < 0) {
		Logger::err() << "Failed to accept connection on " << host.getSocket() << std::endl; 
		throw std::runtime_error("Error, client.");
	}

	epoll_event ev;
	ev.events = EPOLLIN, ev.data.ptr = this;
	if (epoll_ctl(context.getEpoll(), EPOLL_CTL_ADD, _fd, &ev) < 0) {
		Logger::err() << "Failed to add event to epoll" << std::endl;
		close(_fd);
		throw std::runtime_error("Error, client.");
	}

	context.getTimeList().push_front(this);
	context.getRegistery()[_fd] = this;
	timeout_it = context.getTimeList().begin();
	_hostConf = &host.getConfig();
	_lastAlive = std::time(NULL);
}

int	ClientHandler::receiveMsg() {
	size_t	bytes;
	char	buffer[BUFFSIZE];

	bytes = recv(_fd, buffer, BUFFSIZE, MSG_DONTWAIT);
	if (bytes == 0) {
		Logger::err() << "Failed to read msg from client " << _fd << ". Closing connection..." << std::endl;
		return CLT_MSG_ERR;
	}

	if (bytes < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			Logger::err() << "Failed to read msg from client " << _fd << ". Closing connection..." << std::endl;
		return CLT_MSG_ERR;
	}

	try {
		_parser.consume(buffer, bytes);
	}
	catch (HttpParser::HttpRequestParsingException &e) {
		Logger::err() << e.what();
	}

	if (_parser.isComplete()) {
		return CLTMSGEND;
	}
	_lastAlive = std::time(NULL);
	return CLT_MSG_RCV;
}

int	ClientHandler::handleEvent(uint32_t event, WebServ& context) {
	if (event == EPOLLIN) {
		switch (receiveMsg()) {
			case CLTMSGEND:
				if (_request) delete _request;
				_request = _parser.generateRequest();
				break;
			case CLT_MSG_RCV: return CLT_MSG_RCV;
			case CLT_MSG_ERR: return CLTMSGERR;
			default: return 0;
		}
	}

	epoll_event ev;
	ev.events = EPOLLOUT;
	ev.data.ptr = this;
	epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);

	if (event == EPOLLOUT) sendMsg();
}



ClientHandler::~ClientHandler() {
	if (_request) delete _request;
}
