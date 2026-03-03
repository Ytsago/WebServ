#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <cerrno>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

ClientHandler::ClientHandler(): AEventHandler(), _hostConf(NULL) {
}

ClientHandler::ClientHandler(WebServ& context, ServerHandler& host) {
	Logger::record(SETUP) << "Creating new client.";
	if ((_fd = accept(host.getSocket(), NULL, NULL)) < 0) {
		Logger::record(ERROR) << "Failed to accept connection on " << host.getSocket();
		throw std::runtime_error("Error, client.");
	}

	if (addToEpoll(context, EPOLLIN) == EPOLL_CTL_FAIL) {
		throw AEventHandler::HandlerException("Epoll fail");
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

	Logger::record(INFO) << "Receiving a msg from: " << _fd;
	bytes = recv(_fd, buffer, BUFFSIZE, MSG_DONTWAIT);
	if (bytes == 0) {
		Logger::record(ERROR) << "Failed to read msg from client " << _fd << ". Closing connection...";
		return CLT_MSG_ERR;
	}

	if (bytes < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			Logger::record(ERROR) << "Failed to read msg from client " << _fd << ". Closing connection...";
		return CLT_MSG_ERR;
	}

	try {
		_parser.consume(buffer, bytes);
	}
	catch (HttpParser::HttpRequestParsingException &e) {
		Logger::record(ERROR) << e.what();
	}

	if (_parser.isComplete()) {
		Logger::record(INFO) << "Message is complete !";
		return CLT_MSG_END;
	}
	_lastAlive = std::time(NULL);
	return CLT_MSG_RCV;
}

int	ClientHandler::sendMsg() {return 0;}

int	ClientHandler::handleEvent(uint32_t event, WebServ& context) {
	if (event == EPOLLIN) {
		switch (receiveMsg()) {
			case CLT_MSG_END:
				context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);
				if (_request) delete _request;
				_request = _parser.generateRequest();
				break;
			case CLT_MSG_RCV:
				context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);
				return CLT_MSG_RCV;
			case CLT_MSG_ERR:
				return CLT_MSG_ERR;
			default:
				return 0;
		}
	}

	epoll_event ev;
	ev.events = EPOLLOUT;
	ev.data.ptr = this;
	epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);

	if (event == EPOLLOUT) sendMsg();
		return 0;
}



ClientHandler::~ClientHandler() {
	if (_request) delete _request;
}
