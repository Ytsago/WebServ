#include "ClientHandler.hpp"
#include "RequestHandler.hpp"
#include "Logger.hpp"
#include "CgiContainer.hpp"
#include "CgiHandler.hpp"
#include <cerrno>
#include <sys/epoll.h>
#include <sys/signal.h>
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
	_keepAlive = false;
	_pid = -1;
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

void	ClientHandler::setResponse(Response *response) {if (_response) delete _response; _response = response;}

const ServerConfig*	ClientHandler::findHostConf() {
	std::vector<ServerConfig>::const_iterator	it = _hostConf->begin();

	for (; it != _hostConf->end(); it++) {
		std::string	host = it->get_host();
		strLower(host);
		std::string	hostWithPort = host + ":" + int_to_string(it->get_listen_port());
		// Logger::record(DEBUG) << host << ", " << hostWithPort << ", " << _request->getHeaders()["host"] << "\n"
		// 	<< hostWithPort.size() << ", " << _request->getHeaders()["host"].size();
		if (host == _request->getHeaders()["host"] || hostWithPort == _request->getHeaders()["host"])
			return &(*it);
	}
	return &(*_hostConf->begin());
}

int	ClientHandler::receiveMsg(WebServ& context) {
	ssize_t	bytes;
	char	buffer[BUFFSIZE];

	Logger::record(INFO) << "Receiving a msg from: " << _fd;
	bytes = recv(_fd, buffer, BUFFSIZE, MSG_DONTWAIT);
	if (bytes < 0) {
		Logger::record(ERROR) << "Failed to read msg from client " << _fd << ". Closing connection...";
		return CLT_MSG_ERR;
	}
	if (bytes == 0) {
		Logger::record(WARNING) << "Connection closed on client " << _fd << ". Removing handler...";
		return RM_CLT;
	}
	_lastAlive = std::time(NULL);
	context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);

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
        	if (_request->getHeaders()["Connection"] == "keep-alive") {
        		_keepAlive = true;
        	}
        	else
        		_keepAlive = false;
        	_serverConf = findHostConf();
        	// _request->setHost(_serverConf);
        	Logger::record(INFO) << "Processing request...";
            RequestHandler handler(*_serverConf, *_request, context.getEpoll());
            std::string contentType;
			std::string	ext;
        	Logger::record(DEBUG) << "URI: " << _request->getUri();
			if (handler.is_redirection())
			{
        		Logger::record(INFO) << "Request is a redirection";
				return build_response(context.getEpoll()); 
			}
            if (handler.setupUpload(contentType)) 
            {
            	Logger::record(INFO) << "Downloading file...";
                this->_fileHandler = FileHandler(*_request, _serverConf, contentType);
                this->_state = WRITING_BODY;
                if (!this->_request->getBody().empty()) {
                	try {
                    this->_fileHandler.multiparse(this->_request->getBody());
					if (this->_fileHandler.getState() == FileHandler::END)
						return build_response(context.getEpoll());
					} catch (HttpParser::HttpRequestParsingException &e) {
						Logger::record(ERROR) << e.what();
						this->_response = new Response(e.e_status);
						this->_state = SENDING_RESPONSE;
						return CLT_MSG_END;
					}
                }
            }
			else if (handler.get_cgi_ext(ext)) {
				Logger::record(INFO) << "Cgi detected, processing...";
				this->_response = handler.handle_request();
				std::string	path = handler.get_cgi_path();
				t_pipe	fds = CgiHandler::execute_cgi(handler.getServer(), *_request, handler.getLocation(), path, _pid);
				if (_pid == -1)
				{
					Logger::record(ERROR) << "CGI could not be created";
					if (_response) delete _response;
					this->_response = new Response(INTERNAL_SERVER_ERROR);
					this->_state = SENDING_RESPONSE;
					this->activateEpoll(context.getEpoll(), EPOLLOUT);
					return CLT_MSG_END;
				}
				_cgiIn = new CgiContainer(context.getEpoll(), *this, fds.inFd, EPOLLOUT, _pid);
				context.getTimeList().push_front(_cgiIn);
				_cgiIn->setTimeoutIt(context.getTimeList().begin());
				context.getRegistery()[_cgiIn->getSocket()] = _cgiIn;

				_cgiOut = new CgiContainer(context.getEpoll(), *this, fds.outFd, EPOLLIN, _pid);
				context.getTimeList().push_front(_cgiOut);
				_cgiOut->setTimeoutIt(context.getTimeList().begin());
				context.getRegistery()[_cgiOut->getSocket()] = _cgiOut;

				_state = WAITING_CGI;
			}
            else
                return build_response(context.getEpoll());
        }
	}
	else if (this->_state == WRITING_BODY) 
    {
        std::vector<char> chunk(buffer, buffer + bytes);
        try {
        this->_fileHandler.multiparse(chunk);
		} catch (HttpParser::HttpRequestParsingException &e) {
			Logger::record(ERROR) << e.what();
			this->_response = new Response(e.e_status);
			this->_state = SENDING_RESPONSE;
			return CLT_MSG_END;
		}
        if (this->_fileHandler.getState() == FileHandler::END)
            return build_response(context.getEpoll());
    }
	else if (_state == WAITING_CGI) 
	{
		epoll_event ev;
		ev.events = 0;
		ev.data.ptr = this;
		epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
    	// this->_state = SENDING_RESPONSE;
		return CLT_WTG_CGI;
	}
	return CLT_MSG_RCV;
}

int ClientHandler::build_response(int epollFd) 
{
	Logger::record(INFO) << "Building response...";
    RequestHandler handler(*_serverConf, *_request, epollFd);
    if (_response) delete _response;
   	this->_response = handler.handle_request();
	Logger::record(DEBUG) << this->_response->getStatusCode();
    this->_state = SENDING_RESPONSE;
    return CLT_MSG_END;
}

void ClientHandler::handleWrite(WebServ& context) 
{
    if ((this->_state != SENDING_RESPONSE && _state != TIMED_OUT) || !this->_response)  {
    	_keepAlive = false;
    	_state = END;
        return;
    }
    byteVector &resStr = this->_response->get_full_response();
	ssize_t sent = send(this->_fd, resStr.data() + this->_bytesSent, resStr.size() - this->_bytesSent, MSG_NOSIGNAL);
	if (sent > 0) 
	{
		_lastAlive = std::time(NULL);
		context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);
		this->_bytesSent += sent;
		if (this->_bytesSent >= resStr.size())
		{
			Logger::record(INFO) << "Response sent to client " << _fd;
			this->_bytesSent = 0;
			delete _response;
            _response = NULL;
            _parser.reset(); 
            epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.ptr = this;
            epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
			this->_state = END;
		}
	}
	else if (sent <= 0)
	{
		_keepAlive = false;
		this->_state = END;
		this->_bytesSent = 0;
	}
}

void	ClientHandler::resetClient(int epollFd) {
	if (_request) {
		delete (_request);
		_request = NULL;
	}
	if (_response) {
		delete (_response);
		_response = NULL;
	}

	_state = READING_REQUEST;
	_bytesSent = 0;
	_cgiIn = NULL;
	_cgiOut = NULL;
	_parser.reset();

	epoll_event	ev;
	ev.events = EPOLLIN;
	ev.data.ptr = this;
	_serverConf = NULL;
	epoll_ctl(epollFd, EPOLL_CTL_MOD, _fd, &ev);
}

void	ClientHandler::unregisterCgi(CgiContainer* ptr) {
	if (_cgiIn == ptr)
		_cgiIn = NULL;
	else
		_cgiOut = NULL;
}

bool	ClientHandler::sendTimeout(WebServ& context) {
	if (_pid != -1)
		kill(_pid, SIGKILL);
	if (_state != TIMED_OUT) {
		if (_response) delete _response;
		_keepAlive = false;
		if (_pid != -1)
			_response = new Response(GATEWAY_TIMEOUT);
		else
			_response = new Response(REQUEST_TIMEOUT);
		_state = TIMED_OUT;
		epoll_event ev;
		ev.events = EPOLLOUT;
		ev.data.ptr = this;
		epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
		_lastAlive = std::time(NULL);
		return false;
	}
	else {
		return true;
    }
}

int	ClientHandler::handleEvent(uint32_t event, WebServ& context) {
	if (event & EPOLLIN) {
		switch (receiveMsg(context)) {
			case CLT_MSG_END:
				if (_request) delete _request;
				_request = _parser.generateRequest();
				epoll_event ev;
				ev.events = EPOLLOUT;
				ev.data.ptr = this;
				epoll_ctl(context.getEpoll(), EPOLL_CTL_MOD, _fd, &ev);
				if (_response->getStatusCode() >= 400)
					_keepAlive = false;
				break;
			case CLT_MSG_RCV:
				return CLT_MSG_RCV;
			case CLT_MSG_ERR:
				return CLT_MSG_ERR;
			case RM_CLT:
				return RM_CLT;
			case CLT_WTG_CGI:
				break ;
			default:
				return 0;
		}
	}
	if (event & EPOLLOUT) handleWrite(context);
	if (_state == END) {
		if (_keepAlive == false)
			return RM_CLT;
		else
			resetClient(context.getEpoll());
	}
	return CLT_MSG_END;
}

ClientHandler::~ClientHandler() {
	if (_request) delete _request;
	if (_response) delete _response;
}
