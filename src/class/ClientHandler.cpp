#include "ClientHandler.hpp"
#include "RequestHandler.hpp"
#include "Logger.hpp"
#include <cerrno>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

ClientHandler::ClientHandler(): AEventHandler(), _hostConf(NULL) {
}

ClientHandler::ClientHandler(WebServ& context, ServerHandler& host) : _request(NULL), _state(READING_REQUEST) {
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

int	ClientHandler::receiveMsg(WebServ& context) {
	size_t	bytes;
	char	buffer[BUFFSIZE];

	Logger::record(INFO) << "Receiving a msg from: " << _fd;
	bytes = recv(_fd, buffer, BUFFSIZE, MSG_DONTWAIT);
	if (bytes <= 0) {
		Logger::record(ERROR) << "Failed to read msg from client " << _fd << ". Closing connection...";
		return CLT_MSG_ERR;
	}

	if (this->_state == READING_REQUEST) {
		try {
			_parser.consume(buffer, bytes);
		}
		catch (HttpParser::HttpRequestParsingException &e) {
			Logger::record(ERROR) << e.what();
			return CLT_MSG_ERR;
		}
      if (this->_parser.isComplete()) 
        {
        	if (_request) delete(_request);
        	_request = _parser.generateRequest();
        	//TODO choose specific config
        	Logger::record(INFO) << "Processing request...";
            RequestHandler handler((*_hostConf)[0], *_request, context.getEpoll());
            std::string contentType;
            if (handler.setupUpload(contentType)) 
            {
            	Logger::record(INFO) << "Downloading file...";
                //try catch
                this->_fileHandler = new FileHandler(*_request, handler.getLocation(), contentType);
                this->_state = WRITING_BODY;
                if (!this->_request->getBody().empty())
                    this->_fileHandler->multiparse(this->_request->getBody());
            } 
            else
                return build_response(context.getEpoll());
        }
	}
	else if (this->_state == WRITING_BODY) 
    {
        std::vector<char> chunk(buffer, buffer + bytes);
        this->_fileHandler->multiparse(chunk);
        if (this->_fileHandler->getState() == FileHandler::END)
            return build_response(context.getEpoll());
    }
	return CLT_MSG_RCV;
}

int ClientHandler::build_response(int epollFd) 
{
    RequestHandler handler((*_hostConf)[0], *_request, epollFd);
    this->_response = handler.handle_request();
    this->_state = SENDING_RESPONSE;
    return CLT_MSG_END;
    //switch to epollout
}

void ClientHandler::handleWrite() 
{
    if (this->_state != SENDING_RESPONSE || !this->_response) 
        return;
    byteVector &resStr = this->_response->get_full_response();
    ssize_t sent = send(this->_fd, resStr.data() + this->_bytesSent, resStr.size() - this->_bytesSent, 0);
    if (sent > 0) 
    {
        this->_bytesSent += sent;
        if (this->_bytesSent >= resStr.size())
		{
            this->_state = END;
			this->_bytesSent = 0;
		}
    }
    else if (sent == -1)
	{
        this->_state = END;
		this->_bytesSent = 0;
	}
}

int	ClientHandler::handleEvent(uint32_t event, WebServ& context) {
	_lastAlive = std::time(NULL);
	context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);
	if (event == EPOLLIN) {
		switch (receiveMsg(context)) {
			case CLT_MSG_END:
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

	if (event == EPOLLOUT) handleWrite();
	if (_state == END && this->_request->getHeaders()["Connection"] != "keep-alive")
		return RM_CLT;
	return CLT_MSG_END;

}



ClientHandler::~ClientHandler() {
	if (_request) delete _request;
}
