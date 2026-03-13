#include "ClientHandler.hpp"
#include "RequestHandler.hpp"
#include "Logger.hpp"
#include "CgiContainer.hpp"
#include "CgiHandler.hpp"
#include <cerrno>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

ClientHandler::ClientHandler(): AEventHandler(), _hostConf(NULL), _response(NULL) {
}

ClientHandler::ClientHandler(WebServ& context, ServerHandler& host) : _request(NULL), _state(READING_REQUEST), _response(NULL) {
	Logger::record(SETUP) << "Creating new client.";
	if ((_fd = accept(host.getSocket(), NULL, NULL)) < 0) {
		Logger::record(ERROR) << "Failed to accept connection on " << host.getSocket();
		throw std::runtime_error("Error, client.");
	}
	if (addToEpoll(context.getEpoll(), EPOLLIN) == EPOLL_CTL_FAIL) {
		throw AEventHandler::HandlerException("Epoll fail");
	}

	context.getTimeList().push_front(this);
	context.getRegistery()[_fd] = this;
	timeout_it = context.getTimeList().begin();
	_hostConf = &host.getConfig();
	_lastAlive = std::time(NULL);
	_bytesSent = 0;
}

int	ClientHandler::activateEpoll(int epollFd, int event) {
	Logger::record(SETUP) << "Client: " << _fd << "resetting epoll...";
	_state = SENDING_RESPONSE;
	epoll_event ev;
	ev.events = event;
	ev.data.ptr = this;
	epoll_ctl(epollFd, EPOLL_CTL_MOD, _fd, &ev);
	return 1;
}

const HttpRequest&	ClientHandler::getRequest() const { return *_request;}
Response&	ClientHandler::getResponse() { return *_response;}

void	ClientHandler::setResponse(Response *response) {_response = response;}

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
			this->_response = new Response(e.e_status);
			this->_state = SENDING_RESPONSE;
			return CLT_MSG_END;
		}
    	if (this->_parser.isComplete()) 
        {
        	if (_request) delete(_request);
        	_request = _parser.generateRequest();
        	//TODO choose specific config
        	Logger::record(INFO) << "Processing request...";
            RequestHandler handler((*_hostConf)[0], *_request, context.getEpoll());
            std::string contentType;
			std::string	ext;
            if (handler.setupUpload(contentType)) 
            {
            	Logger::record(INFO) << "Downloading file...";
                this->_fileHandler = FileHandler(*_request, handler.getLocation(), contentType);
                this->_state = WRITING_BODY;
                if (!this->_request->getBody().empty())
                    this->_fileHandler.multiparse(this->_request->getBody());
            }
			else if (handler.get_cgi_ext(ext)) {
				Logger::record(INFO) << "Cgi detected, processing...";
				this->_response = handler.handle_request();
				std::string	path = handler.get_cgi_path();
				pid_t pid;
				t_pipe	fds = CgiHandler::execute_cgi(handler.getServer(), *_request, handler.getLocation(), path, pid);
				if (pid == -1)
				{
					Logger::record(ERROR) << "CGI could not be created";
					this->_response = new Response(INTERNAL_SERVER_ERROR);
					if (_response) delete _response;
					this->_state = SENDING_RESPONSE;
					this->activateEpoll(context.getEpoll(), EPOLLOUT);
					return CLT_MSG_END;
				}
				_cgiIn = new CgiContainer(context.getEpoll(), *this, fds.inFd, EPOLLOUT, pid);
				_cgiOut = new CgiContainer(context.getEpoll(), *this, fds.outFd, EPOLLIN, pid);
				_state = WAITING_CGI;
			}
            else
                return build_response(context.getEpoll());
        }
	}
	else if (this->_state == WRITING_BODY) 
    {
        std::vector<char> chunk(buffer, buffer + bytes);
        this->_fileHandler.multiparse(chunk);
        if (this->_fileHandler.getState() == FileHandler::END)
            return build_response(context.getEpoll());
    }
	else if (_state == WAITING_CGI) 
	{
		epoll_event ev;
		ev.events = 0;
		ev.data.ptr = this;
		epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
		context.getTimeList().push_front(_cgiIn);
		context.getTimeList().push_front(_cgiOut);
		context.getRegistery()[_cgiIn->getSocket()] = _cgiIn;
		context.getRegistery()[_cgiOut->getSocket()] = _cgiOut;
    	this->_state = SENDING_RESPONSE;
		return CLT_MSG_END;
	}
	return CLT_MSG_RCV;
}

int ClientHandler::build_response(int epollFd) 
{
	Logger::record(INFO) << "Building response...";
    RequestHandler handler((*_hostConf)[0], *_request, epollFd);
    if (_response) delete _response;
   	this->_response = handler.handle_request();
    this->_state = SENDING_RESPONSE;
    return CLT_MSG_END;
    //switch to epollout
}

void ClientHandler::handleWrite(WebServ& context) 
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
			Logger::record(INFO) << "Response sent to client " << _fd;
			this->_state = END;
			this->_bytesSent = 0;
			delete _response;
            _response = NULL;
            _parser.reset(); 
            epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.ptr = this;
            epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
            this->_state = READING_REQUEST;
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
				epoll_event ev;
				ev.events = EPOLLOUT;
				ev.data.ptr = this;
				epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
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
	else if (event == EPOLLOUT) handleWrite(context);
	if (_state == END && this->_request->getHeaders()["Connection"] != "keep-alive")
		return RM_CLT;
	return CLT_MSG_END;
}



ClientHandler::~ClientHandler() {
	// if (_request) delete _request;
	// if (_response) delete _response;
	// if (_cgi) delete _cgi;
}
