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
void	AEventHandler::setTimeoutIt(std::list<AEventHandler*>::iterator& it) {timeout_it = it;}

AEventHandler::~AEventHandler() {
	if (_fd != -1) close(_fd);
}

#include "AMessage.hpp"


AMessage::AMessage() : flags(0) {
}

AMessage::AMessage(const AMessage &other) : _entryLine(other._entryLine), _body(other._body), _headerField(other._headerField), flags(0){
}

// byteVector	AMessage::buildMsg() const {
// 	byteVector	msg(_entryLine);
//
// 	msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	for (std::map<std::string, std::string>::const_iterator it = _headerField.begin(); it != _headerField.end(); it++) {
// 		msg.insert(msg.end(), it->first.begin(), it->first.end() -1);
// 		msg.push_back(':');
// 		msg.insert(msg.end(), it->second.begin(), it->second.end() -1);
// 		msg.insert(msg.end(), PAT, PAT + 4);
// 	}
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	return msg;
// }

AMessage	&AMessage::operator=(const AMessage &other) {
	std::cout << "Copy assignment operator called" << std::endl;
	if (this != &other) {
		this->_entryLine = other._entryLine;
		this->_body = other._body;
		this->_headerField = other._headerField;
		this->flags = other.flags;
	}
	return (*this);
}

std::string	AMessage::get_content_type()
{
	return (this->_headerField["Content-Type"]);
}

void	AMessage::setFlag(byte flag) {flags |= flag;}
void	AMessage::clearFlag(byte flag) {flags ^= flag;}
const byte&	AMessage::getFlag() const {return flags;}
bool	AMessage::checkFlag(byte flag) const {return (flags & flag) > 0;}
bool	AMessage::eof() const {return (flags & FLAG_EOF) > 0;}
bool	AMessage::fail() const {return (flags & FLAG_FAIL) > 0;}
void	AMessage::clear() {*this = AMessage();}

byteVector&	AMessage::getRaw() {return _raw;}
void	AMessage::setRaw(const byteVector& data) {_raw = data;}

void	AMessage::append(char* buffer, size_t size) {
	if (size == 0)
		setFlag(FLAG_EOF);
	_raw.insert(_raw.end(), buffer, buffer + size);
}

const headerMap&	AMessage::getHeader() const {return _headerField;}
const byteVector&	AMessage::getBody() const {return _body;}

AMessage::~AMessage() {
}
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
	if (bytes < 0)
		return CGI_READ_KO;
	if (bytes == 0)
		return CGI_READ_END;
	Logger::record(INFO) << "Read from CGI: " << buffer;
	_CgiResult.insert(_CgiResult.end(), buffer, buffer + bytes);
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
				break;
			case CGI_READ_END:
				Logger::record(INFO) << "Building CGI response";
				byteVector	response = _parent.getResponse().get_full_response();
				response.insert(response.end(), _CgiResult.begin(), _CgiResult.end());
				_parent.activateEpoll(context.getEpoll(), EPOLLOUT);
				(void)context;
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
#include "CgiHandler.hpp"
#include "Logger.hpp"
#include "CgiContainer.hpp"
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

CgiHandler::CgiHandler() {}

CgiHandler::CgiHandler(const CgiHandler &other) 
{
	(void)other;
}

CgiHandler	&CgiHandler::operator=(const CgiHandler &other) 
{
	(void)other;
	return (*this);
}

CgiHandler::~CgiHandler() {
}

static void	dup_fd(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
	{
		//exception
	}
}

static void	close_pipes(int *pipefd)
{
	int	i;

	i = 0;
	while (i < 4)
	{
		close(pipefd[i]);
		i++;
	}
}

static std::map<std::string, std::string> build_env(const ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path)
{
	std::map<std::string, std::string> env;
	std::string	uri = request.getUri();
	std::string	query;
	size_t		pos;

	pos = uri.find('?');
	query = (pos == std::string::npos) ? "" : uri.substr(pos + 1);
	env["REQUEST_METHOD"] = request.getMethod();
	env["QUERY_STRING"] = query;
	env["CONTENT_LENGTH"] = request.getHeader("Content-Length");
	env["CONTENT_TYPE"] = request.getHeader("Content-Type");
	env["PATH_INFO"] = uri;
	env["PATH_TRANSLATED"] = path;
	env["SCRIPT_NAME"] = location.get_index();
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_NAME"] = server.get_server_name();
	env["SERVER_PORT"] = server.get_listen_port();
	return (env);
}

static void	clear_envp(char **envp)
{
	size_t	i = 0;

	while (envp[i])
	{
		delete [] envp[i];
		i++;
	}
	delete [] envp;
}

static char **map_to_envp(std::map<std::string, std::string> &env)
{
	char	**envp;
	size_t	idx = 0;
	size_t	map_size = env.size();
	std::map<std::string, std::string>::iterator it;

	envp = new char*[map_size + 1];
	for (size_t i = 0; i <= map_size; ++i)
    	envp[i] = NULL;
	try
	{
		for (it = env.begin(); it != env.end(); it++, idx++)
		{
			std::string entry = it->first + "=" + it->second;
			char *field = new char[entry.size() + 1];
			std::copy(entry.begin(), entry.end(), field);
			field[entry.size()] = '\0';
			envp[idx] = field;
		}
	}
	catch(const std::exception& e)
	{
		clear_envp(envp);
		throw;
	}
	return (envp);
}

t_pipe CgiHandler::execute_cgi(const ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path)
{
	std::map<std::string, std::string> env;
	std::vector<char>	body = request.getBody();
	char	**envp;
	int		pid;
	int		pipefd[4];

	env = build_env(server, request, location, path);
	envp = map_to_envp(env);
	for (int i = 0; i < 2; i++)
	{
		if (pipe(pipefd + i * 2) == -1)
		{
			//exception
		}
	}
	/**
	* epoll ctl pipefd[1] and pipefd[2] 
	* -> refacto newconnection in a way that:
	*		Object cgicontainer inherits from ANetContainer
	*		ev.data.ptr = cgicontainer so when epoll wait gives us the fd we know that its a pipefd through ANetContainer::getType()
	*		epoll ctl pipefd[1] on EPOLLOUT
	*		epoll ctl pipefd[2] on EPOLLIN
	*/
	// AEventHandler	*newCgi = new CgiContainer;
	// add_to_epoll(epollFd, pipefd[1], EPOLLOUT, newCgi);
	// add_to_epoll(epollFd, pipefd[2], EPOLLIN, newCgi);
	pid = fork();
	if (pid == -1)
	{
		//exception
	}
	else if (pid == 0)
	{
		dup_fd(pipefd[0], STDIN_FILENO);
		dup_fd(pipefd[3], STDOUT_FILENO);
		close_pipes(pipefd);
		// char **argv = new char*[3];
		// argv[0] = const_cast<char *>(location.get_cgi_path().c_str());
		// argv[1] = const_cast<char *>(path.c_str());
		// argv[2] = NULL;
		// execve(location.get_cgi_path().c_str(), argv, envp);
		std::cout << "CGI TEST\n";
		clear_envp(envp);
		exit(0);
	}
	close (pipefd[0]);
	close (pipefd[3]);
	return (t_pipe) {pipefd[1], pipefd[2]};
}


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
	return addToEpoll(epollFd, event);
}

const HttpRequest&	ClientHandler::getRequest() const { return *_request;}
Response&	ClientHandler::getResponse() { return *_response;}

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
				std::string	path = handler.get_file_path();
   				this->_response = handler.handle_request();
				t_pipe	fds = CgiHandler::execute_cgi(handler.getServer(), *_request, handler.getLocation(), path);
				_cgiIn = new CgiContainer(context.getEpoll(), *this, fds.inFd, EPOLLOUT);
				_cgiOut = new CgiContainer(context.getEpoll(), *this, fds.outFd, EPOLLIN);
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
		epoll_ctl(context.getEpoll(), EPOLL_CTL_DEL, _fd, NULL);
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

void ClientHandler::handleWrite() 
{
    if (this->_state != SENDING_RESPONSE || !this->_response) 
        return;
    byteVector &resStr = this->_response->get_full_response();
		//   if (resStr.size() == _bytesSent || resStr.size()) {
		//   	this->_state = END;
		// this->_bytesSent = 0;
		//   }
    // else {
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
	// }
}

int	ClientHandler::handleEvent(uint32_t event, WebServ& context) {
	_lastAlive = std::time(NULL);
	context.getTimeList().splice(context.getTimeList().begin(), context.getTimeList(), timeout_it);
	if (event == EPOLLIN) {
		switch (receiveMsg(context)) {
			case CLT_MSG_END:
				if (_request) delete _request;
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
	else if (event == EPOLLOUT) handleWrite();
	if (_state == END && this->_request->getHeaders()["Connection"] != "keep-alive")
		return RM_CLT;
	return CLT_MSG_END;
}



ClientHandler::~ClientHandler() {
	// if (_request) delete _request;
	// if (_response) delete _response;
	// if (_cgi) delete _cgi;
}
#include "ConfigParser.hpp"
#include <fstream>

ConfigParser::ConfigParser() : _lineCount(0) {}

ConfigParser::ConfigParser(const char *arg) : _lineCount(0)
{
	this->parse_file(arg);
}

ConfigParser::ConfigParser(const ConfigParser &rhs) : _servers(rhs._servers), _lineCount(rhs._lineCount) {}

ConfigParser	&ConfigParser::operator=(const ConfigParser &rhs)
{
	if (this != &rhs)
	{
		this->_servers = rhs._servers;
		this->_lineCount = rhs._lineCount;
	}
	return (*this);
}

ConfigParser::~ConfigParser() {}

std::vector<ServerConfig>	ConfigParser::get_servers() const {return (this->_servers);};

static void	trim(std::string &str)
{
	const char	*whiteSpaces = " \t\n\r\v\f;";
	size_t		first = str.find_first_not_of(whiteSpaces);
	size_t		last = str.find_last_not_of(whiteSpaces);

	if (first == std::string::npos) 
	{
		str = "";
		return;
	}
	str = str.substr(first, (last - first + 1));
}

void	ConfigParser::parse_file(const char *arg)
{
	std::ifstream	inFile;
	std::string		line;

	inFile.open(arg);
	if (!inFile.is_open())
		throw ConfigException("Couldn't open file", this->_lineCount);
	while (getline(inFile, line))
	{
		this->_lineCount++;
		trim(line);
		if (line.empty() || line[0] == '#') 
			continue;
		if (line == "server {")
			this->_servers.push_back(this->parse_server(inFile));
	}
	inFile.close();
}

ServerConfig	ConfigParser::parse_server(std::ifstream &file)
{
	ServerConfig	server;
	bool			closing_brace = false;
	std::string 	line;
	std::string 	key;
	std::string 	s_value;
	int				i_value;
	size_t			ui_value;

	while (std::getline(file, line)) 
	{
		this->_lineCount++;
		trim(line);
		if (line == "}")
		{
			closing_brace = true;
			break;
		}
		if (line.empty() || line[0] == '#')
			continue;
		std::stringstream ss(line);
		ss >> key;
		if (key == "server")
			throw ConfigException("Missing closing brace", this->_lineCount);
		else if (key == "listen")
		{
			if (!(ss >> i_value))
				throw ConfigException("Invalid listen value", this->_lineCount);
			server.set_listen_port(i_value);
		}
		else if (key == "server_name")
		{
			if (!(ss >> s_value))
				throw ConfigException("Invalid server_name value", this->_lineCount);
			server.set_server_name(s_value);
		}
		else if (key == "host")
		{
			if (!(ss >> s_value))
				throw ConfigException("Invalid host value", this->_lineCount);
			server.set_host(s_value);
		}
		else if (key == "root")
		{
			if (!(ss >> s_value))
				throw ConfigException("Invalid root value", this->_lineCount);
			server.set_root(s_value);
		}
		else if (key == "index")
		{
			if (!(ss >> s_value))
				throw ConfigException("Invalid index value", this->_lineCount);
			server.set_index(s_value);
		}
		else if (key == "error_page")
		{
			if (!(ss >> s_value))
				throw ConfigException("Invalid error_page value", this->_lineCount);
			server.set_error_page(s_value);
		}
		else if (key == "client_max_body_size")
		{
			if (!(ss >> ui_value))
				throw ConfigException("Invalid client max body size value", this->_lineCount);
			server.set_client_mbs(ui_value);
		}
		else if (key == "location")
			server.push_location(this->parse_location(file, line, server));
		else
			throw ConfigException("Invalid server field", this->_lineCount);
	}
	if (!server.is_default_set())
	{
		LocationConfig default_loc;
		default_loc.set_root(server.get_root());
		default_loc.set_index(server.get_index());
		server.set_default_location(default_loc);
		server.set_is_default_set(true);
	}
	if (!closing_brace)
		throw ConfigException("Missing closing brace", this->_lineCount);
	return (server);
}

LocationConfig	ConfigParser::parse_location(std::ifstream &file, std::string header, ServerConfig &server)
{
	LocationConfig		location;
	std::stringstream	ss(header);
	std::string			path;
	std::string			line;
	std::string			key;
	std::string			s_value;
	bool				b_value;
	bool				closing_brace = false;
	bool				has_ext = false;
	bool				has_path = false;

	if (!(ss >> s_value >> path) || path.empty() || path == "{")
		throw ConfigException("Invalid path value", this->_lineCount);
	location.set_path(path);
	if (path == "/")
	{
		server.set_default_location(location);
		server.set_is_default_set(true);
	}
	while (std::getline(file, line)) 
	{
		this->_lineCount++;
		trim(line);
		if (line == "}")
		{
			closing_brace = true;
			break;
		}
		if (line.empty() || line[0] == '#')
			continue ;
		std::stringstream ss_line(line);
		ss_line >> key;
		if (key == "location")
			throw ConfigException("Missing closing brace", this->_lineCount);
		else if (key == "autoindex")
		{
			ss_line >> s_value;
			if (s_value == "on")
				b_value = true;
			else if (s_value == "off")
				b_value = false;
			else
				throw ConfigException("Invalid autoindex value", this->_lineCount);
			location.set_autoindex(b_value);
		}
		else if (key == "root")
		{
			if (!(ss_line >> s_value))
				throw ConfigException("Invalid root value", this->_lineCount);
			location.set_root(s_value);
		}
		else if (key == "index")
		{
			if (!(ss_line >> s_value))
				throw ConfigException("Invalid index value", this->_lineCount);
			location.set_index(s_value);
		}
		else if (key == "allow_methods")
		{
			while (ss_line >> s_value)
			{
				if (s_value != "GET" && s_value != "POST" && s_value != "DELETE")
					throw ConfigException("Invalid method value", this->_lineCount);
				location.push_method(s_value);
			}
		}
		else if (key == "cgi_ext")
		{
			if (!(ss_line >> s_value))
				throw ConfigException("Invalid cgi_ext value", this->_lineCount);
			location.set_cgi_ext(s_value);
			location.set_is_cgi(true);
			has_ext = true;
		}
		else if (key == "cgi_path")
		{
			if (!(ss_line >> s_value))
				throw ConfigException("Invalid cgi_path value", this->_lineCount);
			location.set_cgi_path(s_value);
			location.set_is_cgi(true);
			has_path = true;
		}
		else
			throw ConfigException("Invalid location field", this->_lineCount);
	}
	if (location.is_cgi() && (!has_ext || !has_path))
		throw ConfigException("CGI configuration is incomplete: both 'cgi_ext' and 'cgi_path' are required", this->_lineCount);
	if (!closing_brace)
		throw ConfigException("Missing closing brace", this->_lineCount);
	return (location);
}

std::ostream &operator<<(std::ostream &out, const ConfigParser &conf)
{
	std::vector<ServerConfig>	servers = conf.get_servers();
	for (size_t i = 0; i < servers.size(); i++)
		out << servers[i];
	return (out);
}
#include "FileHandler.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

FileHandler::FileHandler() : _fileFd(-1) {}

FileHandler::FileHandler(HttpRequest &request, LocationConfig &location, std::string &content_type) :
	// _request(request),
	// _location(location),
	// _contentType(content_type),
	_state(SEARCH_BOUNDARY),
	_fileFd(-1)
{
	(void)location;
	(void)request;
	(void)content_type;
	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		std::string h_content = request.getHeader("Content-Type");
		size_t pos = h_content.find("boundary=");
		if (pos != std::string::npos)
			this->_boundary = "--" + h_content.substr(pos + 9);
	}
	// Le chemin d'upload devrait idéalement venir de la configuration de la Location
	this->_uploadPath = "./website/uploads/"; 
}

FileHandler&	FileHandler::operator=(const FileHandler& other) {
	if (this != &other) {
		_state = other._state;
		_fileFd = other._fileFd;
		_boundary = other._boundary;
		_buffer = other._buffer;
		_filename = other._filename;
		_uploadPath = other._uploadPath;
	}
	return (*this);
}

FileHandler::~FileHandler() 
{
	if (_fileFd != -1)
		close(_fileFd);
}

int	FileHandler::getState() { return (this->_state); }

static std::string sanitize_filename(std::string filename)
{
	std::stringstream	ss;
	std::string 		sanitized;
	size_t				last_slash = filename.find_last_of("/");

	sanitized = (last_slash == std::string::npos) ? filename : filename.substr(last_slash + 1);
	if (sanitized.empty())
		sanitized = "default";
	ss << time(NULL) << "_" << sanitized;
	sanitized = ss.str();
	return (sanitized);
}

static std::string extract_filename(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
	std::string filename;
	std::string header(begin, end);
	size_t 		pos = header.find("filename=\"");

	if (pos == std::string::npos)
		return sanitize_filename("unknown_file");
	size_t first = pos + 10;
	size_t last = header.find("\"", first);
	filename = header.substr(first, last - first);
	return (sanitize_filename(filename));
}

// static void	print_buf(std::vector<char> &buf)
// {
// 	for (std::vector<char>::iterator ite = buf.begin(); ite != buf.end(); ite++)
// 		std::cout << *ite;
// 	std::cout << "+++++++++++++++++++++\n";
// }

//TODO check if FD > 0
void FileHandler::multiparse(const std::vector<char> &chunk) 
{
	this->_buffer.insert(this->_buffer.end(), chunk.begin(), chunk.end());
	while (true) 
	{
		if (this->_state == SEARCH_BOUNDARY) 
		{
			std::vector<char>::iterator it = std::search(this->_buffer.begin(), this->_buffer.end(), this->_boundary.begin(), this->_boundary.end());
			if (it == this->_buffer.end()) 
				break;
			this->_buffer.erase(this->_buffer.begin(), it + this->_boundary.size());
			this->_state = PARSE_HEADERS;
		}
		if (this->_state == PARSE_HEADERS) 
		{
			const char *del = "\r\n\r\n";
			std::vector<char>::iterator it = std::search(this->_buffer.begin(), this->_buffer.end(), del, del + 4);
			if (it == this->_buffer.end()) 
				break;
			this->_filename = extract_filename(this->_buffer.begin(), it);
			this->_filename = this->_uploadPath + this->_filename;
			this->_fileFd = open(this->_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			Logger::record(INFO) << "File: " << _filename.c_str() << " fd: " << _fileFd;
			this->_buffer.erase(this->_buffer.begin(), it + 4);
			this->_state = WRITING_DATA;
		}
		if (this->_state == WRITING_DATA)
		{
			const char *end_boundary = "--";
			std::vector<char>::iterator it = std::search(this->_buffer.begin(),this-> _buffer.end(), this->_boundary.begin(), this->_boundary.end());
			if (it != _buffer.end())
			{
				size_t boundary_offset = std::distance(this->_buffer.begin(), it);
				if (boundary_offset >= 2)
					write(this->_fileFd, &(*this->_buffer.begin()), boundary_offset - 2);
				close(this->_fileFd);
				bool is_end = false;
				if (std::distance(it + this->_boundary.size(), this->_buffer.end()) >= 2)
				{
					if (std::equal(end_boundary, end_boundary + 2, it + this->_boundary.size()))
						is_end = true;
				}
				else if (std::distance(it + this->_boundary.size(), this->_buffer.end()) < 2)
					break;
				this->_buffer.erase(this->_buffer.begin(), it);
				if (is_end) 
				{
					this->_state = END;
					this->_buffer.clear();
				} 
				else
					this->_state = SEARCH_BOUNDARY;
			}
			else 
			{
				if (this->_buffer.size() > this->_boundary.size() + 4) 
				{
					size_t write_size = this->_buffer.size() - (this->_boundary.size() + 4);
					write(this->_fileFd, &(*this->_buffer.begin()), write_size);
					this->_buffer.erase(this->_buffer.begin(), this->_buffer.begin() + write_size);
				}
				break; 
			}
		}
		if (this->_state == END)
			break;
	}
}
#include "HeaderMap.hpp"

HeaderMap::HeaderMap() {
}

std::string	HeaderMap::operator[](std::string key) const {
	strLower(key);

	for (size_t i = 0; i < m_headers.size(); i++) {
		if (m_headers[i].first == key)
			return m_headers[i].second;
	}
	return "";
}

HeaderMap&	HeaderMap::operator=(const HeaderMap& other) {
	if (this != &other)
		m_headers = other.m_headers;
	return *this;
}

void	HeaderMap::add(std::string key, std::string value) {
	strLower(key);
	
	for (size_t i = 0; i < m_headers.size(); i++) {
		if (m_headers[i].first == key) {
			m_headers[i].second += ", " + value;
			return ;
		}
	}
	m_headers.push_back(std::make_pair(key, value));
}

void	HeaderMap::swap(HeaderMap& other) {
	m_headers.swap(other.m_headers);
}

void	HeaderMap::reserve(int x) {
	m_headers.reserve(x);
}

void	HeaderMap::clear() {
	m_headers.clear();
}

HeaderMap::iterator	HeaderMap::begin() {
	return m_headers.begin();
}

HeaderMap::iterator	HeaderMap::end() {
	return m_headers.end();
}

HeaderMap::~HeaderMap() {

}
#include "HttpParser.hpp"
#include "StatusCode.hpp"
#include <algorithm>
#include "Logger.hpp"
#include <cstddef>
#include "utils.hpp"

HttpParser::HttpParser() : m_state(REQUEST_LINE), m_type(NONE), m_cursor(0), m_bodySize(0), m_readBuf() {
	m_readBuf.reserve(8192);
	m_headers.reserve(20);
}

std::string	HttpParser::getHeader(const std::string& key) const {return m_headers[key];}

bool	HttpParser::isComplete() const {return m_state == COMPLETE;}

void	HttpParser::consume(const char* data, size_t len) {
	if (len + m_readBuf.size() > 8192 && m_state != BODY)
		throw HttpRequestParsingException(REQUEST_HEADER_FIELD_TOO_LARGE);
	m_readBuf.insert(m_readBuf.end(), data, data + len);

	bool	again = true;
	while (again) {
		switch (m_state) {
			case REQUEST_LINE: again = parseRequestLine(); break;
			case HEADER: again = parseHeader(); break;
			case BODY: again = parseBody(); break;
			case COMPLETE: again = false; break;	//TODO add a check for Host
			case CHUNKEDBODY: throw HttpRequestParsingException(NOT_IMPLEMENTED);
			default: again = false; break;
		}
	}
}

bool	HttpParser::parseRequestLine() {
	std::vector<char>::iterator	itStart = m_readBuf.begin() + m_cursor;
	std::vector<char>::iterator	itEndLine = std::search(itStart, m_readBuf.end(), "\r\n", ("\r\n") +2);

	if (itEndLine == m_readBuf.end()) {
		if (m_readBuf.size() - m_cursor > 2048)
			throw HttpRequestParsingException(URI_TOO_LONG);
		return false;
	}
	
	if (std::distance(itStart, itEndLine) > 2048)
		throw HttpRequestParsingException(URI_TOO_LONG);

	std::vector<char>::iterator itFirstSpace, itSecondSpace;
	itFirstSpace = std::find(itStart, itEndLine, ' ');
	if (itFirstSpace == itEndLine)
		throw HttpRequestParsingException(BAD_REQUEST);
	itSecondSpace = std::find(itFirstSpace + 1, itEndLine, ' ');
	if (itSecondSpace == itEndLine)
		throw HttpRequestParsingException(BAD_REQUEST);

	m_method.assign(itStart, itFirstSpace);
	m_path.assign(itFirstSpace + 1, itSecondSpace);
	std::string	version(itSecondSpace + 1, itEndLine);

	if (m_method.empty() || m_path.empty() || version.empty())
		throw HttpRequestParsingException(BAD_REQUEST);
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		throw HttpRequestParsingException(HTTP_VERSION_NOT_SUPPORTED);

	m_cursor += std::distance(itStart, itEndLine + 2);
	m_state = HEADER;
	return true;
}

void	HttpParser::parseMediaType(std::string& rawMedia) {
	size_t	slashPos = rawMedia.find('/');

	if (slashPos == std::string::npos || slashPos == rawMedia.size() -1 || slashPos == 0) {
		Logger::record(ERROR) << "1";
		throw HttpRequestParsingException(BAD_REQUEST);
	}

	std::string	media = rawMedia.substr(0, slashPos);
	m_subtype = rawMedia.substr(slashPos +1);
	trim(media);
	trim(m_subtype);

	if (m_subtype.empty()) {
		Logger::record(ERROR) << "2";
		throw HttpRequestParsingException(BAD_REQUEST);
	}
	if (media == "text") m_type = TEXT;
	else if (media == "application") m_type = APPLICATION;
	else if (media == "multipart") m_type = MULTIPART;
	else throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
}

std::string	extractBoundary(std::string& header) {
	std::string tmp(header);
	strLower(tmp);

	size_t	boundIndex = tmp.find("boundary=");
	if (boundIndex == std::string::npos) return "";

	size_t	len;
	size_t	boundEnd = tmp.find(" \t;", boundIndex +9);
	if (boundEnd == std::string::npos)
		len = std::string::npos;
	else
		len = boundEnd - (boundIndex + 9);

	std::string	boundary = header.substr(boundIndex + 9, len);
	if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size() -1] == '"')
		boundary = boundary.substr(1, boundary.size() -2);
	return (boundary);
}

void	HttpParser::processHeader() {
	std::string header;
	if (!(header = getHeader("transfer-encoding")).empty()) {
		if (header != "chunked")
			throw HttpRequestParsingException(NOT_IMPLEMENTED);
		m_state = CHUNKEDBODY;
	}
	else if (!(header = getHeader("content-length")).empty()) {
		char*	endptr;
		long	val;
		val = std::strtol(header.c_str(), &endptr, 10);
		if (*endptr != '\0' || val < 0 || val > MAX_BODY_SIZE) {
			Logger::record(ERROR) << "3";
			throw HttpRequestParsingException(BAD_REQUEST);
		}
		m_contentLength = static_cast<size_t>(val);
		if (m_contentLength > 0) {
			m_state = BODY;
		}
		else
			m_state = COMPLETE;
	}
	else {
		m_state = COMPLETE;
		return ;
	}
	if (!(header = getHeader("content-type")).empty()) {
		size_t	semiColon = header.find(';');
		std::string	media = header.substr(0, semiColon);
		if (media.empty())
			return ;
		strLower(media); parseMediaType(media);
		switch (m_type) {
			case TEXT:
				if (m_subtype.empty()) {
					// throw HttpRequestParsingException(BAD_REQUEST);
					break;
				}
				break;
			case APPLICATION:
				if (m_subtype == "x-www-form-urlencoded") {
					throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
					break ;
				}
				else if (m_subtype == "octet-stream")
					m_type = TEXT;
				else
					throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
				break ;
			case MULTIPART:
				if (m_subtype != "form-data")
					throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
				break ;
			default:
				throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
		}
	}
	return ;
}

//TODO add check for host
bool	HttpParser::parseHeader() {
	std::vector<char>::iterator	itStart = m_readBuf.begin() + m_cursor;
	std::vector<char>::iterator	itEndLine = std::search(itStart, m_readBuf.end(), "\r\n", ("\r\n") +2);

	if (itEndLine == m_readBuf.end())
		return false;

	if (itEndLine == itStart) {
		m_cursor += 2;
		processHeader();
		return true;
	}

	std::vector<char>::iterator	itSeparator = std::find(itStart, itEndLine, ':');
	if (itSeparator == itEndLine) {
		Logger::record(ERROR) << "5";
		throw HttpRequestParsingException(BAD_REQUEST);
	}

	std::string	key(itStart, itSeparator);
	if (key.empty()) {
		Logger::record(ERROR) << "6";
		throw HttpRequestParsingException(BAD_REQUEST);
	}

	std::string	value(itSeparator + 1, itEndLine);
	size_t	firstSpace = value.find_first_not_of(' ');
	if (firstSpace == std::string::npos)
		value = "";
	else {
		size_t secondSpace = value.find_last_not_of(' ');
		value = value.substr(firstSpace, secondSpace - firstSpace + 1);
	}
	strLower(key);
	if (canBeLowered(key))
		strLower(value);

	std::vector<std::pair<std::string, std::string> >::iterator	it;
	for (it = m_headers.begin(); it != m_headers.end(); it++)
		if (it->first == key)
			break;
	if (it == m_headers.end())
		m_headers.add(key, value);
	else 
		it->second += ", " + value;

	m_cursor += std::distance(itStart, itEndLine + 2);
	return true;
}

bool	HttpParser::parseBody() {
	size_t	bytesNeeded = m_contentLength - m_body.size();
	size_t	available = m_readBuf.size() - m_cursor;

	if (available > 0) {
		size_t	toCopy = (available < bytesNeeded) ? available : bytesNeeded;
		// switch (m_type) {
		// 	case TEXT:
		// 		m_body.insert(m_body.end(), m_readBuf.begin() + m_cursor, m_readBuf.begin() + toCopy+ m_cursor);
		// 		break ;
		// 	case APPLICATION:
		// 		throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
		// 	case MULTIPART:
		// 		break ;
		// 	default:
		// 		throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
		// }
		m_body.insert(m_body.end(), m_readBuf.begin() + m_cursor, m_readBuf.begin() + toCopy+ m_cursor);
		m_cursor += toCopy;
		m_bodySize += toCopy;
		if (m_type == MULTIPART)
			m_state = COMPLETE;
	}

	if (m_bodySize == m_contentLength) {
		m_state = COMPLETE;
		return true;
	}

	if (m_cursor >= m_readBuf.size()) {
		m_cursor = 0;
		m_readBuf.clear();
	}
	return false;
}

void	HttpParser::reset() {
	if (m_cursor == m_readBuf.size()) m_readBuf.clear();
	else m_readBuf.erase(m_readBuf.begin(), m_readBuf.begin() + m_cursor);

	m_cursor = 0;
	m_contentLength = 0;
	m_state = REQUEST_LINE;
	m_type = NONE;
	m_bodySize = 0;
}

HttpRequest*	HttpParser::generateRequest() {
	HttpRequest*	request = new HttpRequest;

	request->_method.swap(m_method);
	request->_body.swap(m_body);
	request->_header.swap(m_headers);
	request->_uri.swap(m_path);
	request->_contentLength = m_contentLength;

	this->reset();
	return request;
}

HttpParser::~HttpParser() {

}

const char*	HttpParser::HttpRequestParsingException::what() const throw() {
	return g_status_map[e_status].c_str(); 
}
#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _contentLength(0) {
	_header.reserve(10);
}

HttpRequest::HttpRequest(const HttpRequest& other) :
	_method(other._method),
	_uri(other._uri),
	_header(other._header),
	_body(other._body),
	_contentLength(other._contentLength) {}

HttpRequest&	HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) {
		this->_method = other._method;
		this->_uri = other._uri;
		this->_header = other._header;
		this->_body = other._body;
		this->_contentLength = other._contentLength;
	}
	return *this;
}

std::string	HttpRequest::getHeader(const std::string& key) const 
{
	return this->_header[key];
}

const std::string&	HttpRequest::getMethod() const {return _method;}
const std::string&	HttpRequest::getUri() const {return _uri;}
const HeaderMap& HttpRequest::getHeaders() const {return _header;}
const std::vector<char>&	HttpRequest::getBody() const {return _body;}

HttpRequest::~HttpRequest() {

};
#include "LocationConfig.hpp"

LocationConfig::LocationConfig() 
: _path(""),
  _root(""),
  _index(""),
  _methods(0),
  _autoindex(0),
  _cgiExt(""),
  _cgiPath(""),
  _isCgi(0) {}

LocationConfig::LocationConfig(const LocationConfig &rhs)
: _path(rhs._path),
  _root(rhs._root),
  _index(rhs._index),
  _methods(rhs._methods),
  _autoindex(rhs._autoindex),
  _cgiExt(rhs._cgiExt),
  _cgiPath(rhs._cgiPath),
  _isCgi(rhs._isCgi) {}

LocationConfig	&LocationConfig::operator=(const LocationConfig &rhs)
{
	if (this != &rhs)
	{
		this->_path = rhs._path;
		this->_root = rhs._root;
		this->_index = rhs._index;
		this->_methods = rhs._methods;
		this->_autoindex = rhs._autoindex;
		this->_cgiExt = rhs._cgiExt;
		this->_cgiPath = rhs._cgiPath;
		this->_isCgi = rhs._isCgi;
	}
	return (*this);
}

LocationConfig::~LocationConfig() {}

std::string					LocationConfig::get_path() const {return (this->_path);};
std::string					LocationConfig::get_root() const {return (this->_root);};
std::string					LocationConfig::get_index() const {return (this->_index);};
std::vector<std::string>	LocationConfig::get_methods() const {return (this->_methods);};
bool						LocationConfig::get_autoindex() const {return (this->_autoindex);};
std::string					LocationConfig::get_cgi_ext() const {return (this->_cgiExt);};
std::string					LocationConfig::get_cgi_path() const {return (this->_cgiPath);};
bool						LocationConfig::is_cgi() const {return (this->_isCgi);};

void	LocationConfig::set_path(std::string value) {this->_path = value;};
void	LocationConfig::set_root(std::string value) {this->_root = value;};
void	LocationConfig::set_index(std::string value) {this->_index = value;};
void	LocationConfig::push_method(std::string value) {this->_methods.push_back(value);};
void	LocationConfig::set_autoindex(bool value) {this->_autoindex = value;};
void	LocationConfig::set_cgi_ext(std::string value) {this->_cgiExt = value;};
void	LocationConfig::set_cgi_path(std::string value) {this->_cgiPath = value;};
void	LocationConfig::set_is_cgi(bool value) {this->_isCgi = value;};

std::ostream &operator<<(std::ostream &out, const LocationConfig &location)
{
	out << "location: " << '\n' 
	<< "\tis_cgi: " << (location.is_cgi() ? "true" : "false") << '\n'
	<< "\troot: " << location.get_root() << '\n'
	<< "\tindex: " << location.get_index() << '\n'
	<< "\tpath: " << location.get_path() << '\n';
	if (location.get_methods().size() > 0)
	{
		std::vector<std::string> methods = location.get_methods();
		out << "\tmethods: ";
		for (size_t i = 0; i < methods.size(); i++)
			out << methods[i] << ' ';
		out << '\n';
	}
	out << "\tautoindex: " << location.get_autoindex() << '\n';
	if (location.is_cgi())
	{
		out << "\tcgi_ext: " << location.get_cgi_ext() << '\n'
		<< "\tcgi_path: " << location.get_cgi_path() << '\n';
	}
	return (out);
}#include "Logger.hpp"

LogLine::LogLine(std::ostream& os) : _os(os) {}

LogLine::~LogLine() {
	if (Logger::getColor()) _os << RESET << std::endl;
	else _os << std::endl;
}

//-------------Logger-------------//

Logger::Logger(): logs(std::cout), errLogs(std::cerr), color(true) {}
Logger::Logger(std::ostream& logStream, std::ostream& errStream): logs(logStream), errLogs(errStream) {}
bool	Logger::getColor() {return Logger::getInstance().color;}

Logger& Logger::getInstance() {
	static Logger _instance(std::cout, std::cerr);
	return (_instance);
}

std::ostream&	Logger::out() {
	return Logger::getInstance().logs;
}

std::ostream&	Logger::err() {
	return Logger::getInstance().errLogs;
}

std::ostream&	Logger::log(logLevel level) {
	Logger& instance = getInstance();
	std::ostream&	os = (level == ERROR) ? instance.err() : instance.out();

	if (instance.color) {
		switch (level) {
			case INFO: os << FG_AQUA; break;
			case SETUP: os << FG_SALMON;  break;
			case SUCCESS: os << FG_LIME; break;
			case WARNING: os << FG_AMBER; break;
			case ERROR: os << FG_TOMATO; break;
			case DEBUG: os << FG_CHARCOAL; break;
		}
	}

	switch (level) {
		case INFO: os << "[INFO]: "; break;
		case SETUP: os << "[SETUP]: "; break;
		case SUCCESS: os << "[DONE]: "; break;
		case WARNING: os << "[WARNING]: "; break;
		case ERROR: os << "[ERROR]: "; break;
		case DEBUG: os << "[DEBUG]: "; break;
	}
	return os;
}

LogLine	Logger::record(logLevel level) {
	return LogLine(log(level));
}

void	Logger::upColor() {Logger::getInstance().color = true;}
void	Logger::downColor() {Logger::getInstance().color = false;}

Logger::~Logger() {

}
#include "Request.hpp"
#include <cstring>
#include <sstream>
#include <algorithm>

// static const byte PATTERN[4] = {'\r', '\n', '\r', '\n'};

Request::Request() : AMessage() {
}

Request::Request(const Request &other) : AMessage(other) {
}

Request	&Request::operator=(const Request &other) {
	this->AMessage::operator=(other);
	return *this;
}
/*
//[TODO] Continue parsing here
void	Request::processEntry() {
	byteVector&	raw = getRaw();
	std::string*	info[3] = {&_method, &_uri, &_version};
	size_t	j = 0;

	for (size_t i = 0; i + 3 < raw.size(); i++) {
		if (std::memcmp(&raw[i], PATTERN, 2) == 0) {
			setFlag(FLAG_ENTRY);
			if (info[0]->empty() || info[1]->empty() || info[2]->empty())
				// setFlag(FLAG_FAIL);
			return ;
		}
		if (raw[i] == ' ')
			j++;
		if (j > 2) {
			// setFlag(FLAG_FAIL);
			return ;
		}
		else
			info[j] += raw[i];
	}
}
*/

void	Request::processEntry() {
	byteVector	&raw = this->getRaw();
	byteVector::iterator	endLine;

	endLine = std::find(raw.begin(), raw.end(), '\r');
	if (endLine == raw.end() || endLine +1 == raw.end()) {
		return ;
	}
	if (endLine + 1 != raw.end() && *(endLine + 1) != '\n') {
		return ;
	}

	//TODO Add security check for bad input. Also, whitespace may be consecutive in final version
	//Check that i does not exceed 2
	std::stringstream	iss(std::string(raw.begin(), endLine));
	iss >> _method >> _uri >> _version;
	this->setFlag(FLAG_ENTRY);
	raw.erase(raw.begin(), endLine + 2);
}

void	Request::processHeader() {
	byteVector	&raw = this->getRaw();
	byteVector::iterator	endLine;

	endLine = std::find(raw.begin(), raw.end(), '\r');
	if (endLine == raw.end() || endLine +1 == raw.end()) {
		// this->setFlag(FLAG_EOF);
		return ;
	}

	if (endLine + 1 != raw.end() && *(endLine + 1) != '\n') {
		this->setFlag(FLAG_FAIL);
		return ;
	}

	if (endLine == raw.begin()) {
		raw.erase(raw.begin(), raw.begin() + 2);
		this->setFlag(FLAG_HEADER);
		return ;
	}

	std::string	field(raw.begin(), endLine);
	size_t	commaPos = field.find(':');
	
	if (commaPos == field.npos) {
		setFlag(FLAG_FAIL);
		return ;
	}

	this->_headerField[field.substr(0, commaPos)] = field.substr(commaPos + 1, field.size());
	raw.erase(raw.begin(), endLine + 2);
}

//TODO Add check using content lenght ex :
// Content-Lenght: 200, body + raw > 200 -> ERROR
void	Request::processBody() {
	byteVector&	raw = getRaw();

	this->_body.insert(_body.end(), raw.begin(), raw.end());
	raw.erase(raw.begin(), raw.end());
	return ;
}

bool	Request::processMsg() {
	const byte& flag = this->getFlag();

	while (!(checkFlag(FLAG_EOF | FLAG_FAIL))) {
		size_t prevSize = getRaw().size();
		if (this->fail())
			return 1;
		if ((flag & FLAG_ENTRY) == 0)
			this->processEntry();
		else if ((flag & FLAG_HEADER) == 0)
			this->processHeader();
		else if ((flag & FLAG_BODY) == 0) {
			this->processBody();
		}

		if (_method == "GET" && (flag & FLAG_HEADER) > 0) {
			setFlag(FLAG_EOF);
			break ;
		}
		if (prevSize == getRaw().size())
			break ;
	}
	//TODO remove this line (it's for testing purpose)
	return 0;
}

std::string	Request::get_method() const {return _method;}
std::string	Request::get_uri() const {return _uri;}
std::string	Request::get_version() const {return _version;}

Request::~Request() {
}

std::ostream&	operator<<(std::ostream& out, const Request& el) {
	out << "Entry Line: \n";
	out << el.get_method() << " " << el.get_uri() << " " << el.get_version() << "\n";

	out << "\nHeader fields: \n";
	for (headerMap::const_iterator it = el.getHeader().begin(); it != el.getHeader().end(); it++) {
		out << it->first << ": " << it->second << "\n";
	}

	out << "\nBody: \n";
	out << std::string(el.getBody().begin(), el.getBody().end());
	return out;
}
#include "RequestHandler.hpp"
#include "Response.hpp"
#include "CgiHandler.hpp"
#include "StatusCode.hpp"
#include "FileHandler.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <algorithm>
#include <map>
#include <sstream>

RequestHandler::RequestHandler(const ServerConfig &server, HttpRequest &request, int epollFd) :
	_server(server),
	_request(request),
	_location(),
	_epollFd(epollFd) {}

RequestHandler::RequestHandler(const RequestHandler &other) :
	_server(other._server),
	_request(other._request),
	_location(other._location),
	_epollFd(other._epollFd) {}

// RequestHandler	&RequestHandler::operator=(const RequestHandler &other) 
// {
// 	if (this != &other)
// 	{
// 		this->_server = other._server;
// 		this->_request = other._request;
// 		this->_location = other._location;
// 		this->_epollFd = other._epollFd;
// 	}
// 	return (*this);
// }

RequestHandler::~RequestHandler() {}

LocationConfig	&RequestHandler::getLocation() {return (this->_location);}
const ServerConfig	&RequestHandler::getServer() {return (this->_server);}

static bool check_path_correspondance(std::vector<std::string> &uri_blocks, std::vector<std::string> &loc_blocks, size_t block_nb)
{
	for (size_t i = 0; i < block_nb; i++)
	{
		if (uri_blocks[i].compare(loc_blocks[i]) != 0)
			return false;
	}
	return true;
}

void	RequestHandler::find_corresponding_location()
{
	std::vector<LocationConfig>::iterator	it;
	std::vector<LocationConfig>	server_locations = this->_server.get_locations();
	std::vector<LocationConfig>	potential_locations;
	std::string					uri = this->_request.getUri();
	std::string					buffer;
	std::vector<std::string>	uri_blocks;
	size_t						block_nb;
	char						del = '/';
	size_t						loc_size = 0;
	size_t						longest_loc_size = 0;

	std::stringstream 	ssu(uri);
	while (getline(ssu, buffer, del))
	{
		if (!buffer.empty())
			uri_blocks.push_back(buffer);
	}
	for (it = server_locations.begin(); it != server_locations.end(); it++)
	{
		std::stringstream 	ssl(it->get_path());
		std::vector<std::string>	loc_blocks;
		while (getline(ssl, buffer, del))
		{
			if (!buffer.empty())
				loc_blocks.push_back(buffer);
		}
		if (loc_blocks.size() > uri_blocks.size())
			continue;
		block_nb = loc_blocks.size();
		if (check_path_correspondance(uri_blocks, loc_blocks, block_nb))
			potential_locations.push_back(*it);
	}
	this->_location = this->_server.get_default_location();
	for (it = potential_locations.begin(); it != potential_locations.end(); it++)
	{
		loc_size = it->get_path().size();
		if (loc_size > longest_loc_size)
		{
			longest_loc_size = loc_size;
			this->_location = *it;
		}
	} 
}

std::string	RequestHandler::get_file_path()
{
	std::string	uri = this->_request.getUri();
	std::string	root;
	std::string	index;
	std::string path;

	root = this->_location.get_root().empty() ? this->_server.get_root() : this->_location.get_root();
    if (!root.empty() && root[root.size() - 1] != '/')
		root += '/';
	std::string loc_path = this->_location.get_path();
	std::string part_after_loc = uri.substr(loc_path.length());
    if (part_after_loc.empty() || part_after_loc == "/") 
    {
        index = this->_location.get_index().empty() ? this->_server.get_index() : this->_location.get_index();
        path = root + index;
    } 
    else 
    {
        if (part_after_loc[0] == '/')
            part_after_loc.erase(0, 1);
        path = root + part_after_loc;
    }
	return (path); 
}

static bool	check_if_method_allowed(LocationConfig &location, std::string method)
{
	std::vector<std::string> 	allowed_methods = location.get_methods();
	if (allowed_methods.empty()) return true;
	for (size_t i = 0; i < allowed_methods.size(); i++)
	{
		if (allowed_methods[i].compare(method) == 0)
			return true;
	}
	return false;
}

bool RequestHandler::setupUpload(std::string &content_type)
{
	this->find_corresponding_location();
	if (this->_request.getMethod() != "POST")
		return false;
	if (!check_if_method_allowed(this->_location, "POST"))
		return false;
	return this->get_upload_type(content_type);
}

Response* RequestHandler::handle_request()
{
	std::string method = _request.getMethod();
	if (method == "GET") 
		return this->build_get_response();
	if (method == "POST") 
		return this->build_post_response();
	if (method == "DELETE") 
		return this->build_delete_response();
	return new Response(METHOD_NOT_ALLOWED);
}

Response* RequestHandler::build_get_response()
{
	std::string		path;
	byteVector		file;
	std::string		ext;

	this->find_corresponding_location();
	path = this->get_file_path();
	if (!check_if_method_allowed(this->_location, "GET"))
		return new Response(METHOD_NOT_ALLOWED);
	if (this->get_cgi_ext(ext))
		return new Response(OK, byteVector(), path, true);
	file = GetFile(path);
	if (file.empty() || access(path.c_str(), F_OK) != 0)
		return new Response(NOT_FOUND);
	return new Response(OK, file, path, true);
}

Response* RequestHandler::build_post_response()
{
	std::string		ext;
	std::string		path;
	byteVector		emptyBody;

	this->find_corresponding_location();
	path = this->get_file_path();
	if (!check_if_method_allowed(this->_location, "POST"))
		return new Response(METHOD_NOT_ALLOWED);
	if (this->get_cgi_ext(ext))
		return new Response(OK, emptyBody, path, true);
	return new Response(CREATED, emptyBody, path, true);
}

Response* RequestHandler::build_delete_response()
{
    std::string path;

    this->find_corresponding_location();
    path = this->get_file_path();
    if (!check_if_method_allowed(this->_location, "DELETE"))
        return new Response(METHOD_NOT_ALLOWED);
    if (access(path.c_str(), F_OK) != 0)
        return new Response(NOT_FOUND);
    if (unlink(path.c_str()) == 0)
        return new Response(NO_CONTENT);
    else
        return new Response(INTERNAL_SERVER_ERROR);
}

bool	RequestHandler::get_upload_type(std::string &content_type)
{
	std::string req_ct = this->_request.getHeader("Content-Type");
	if (req_ct.find("multipart/form-data") != std::string::npos) 
	{
		content_type = "multipart/form-data";
		return true;
	}
	if (req_ct.find("application/octet-stream") != std::string::npos) {
		content_type = "application/octet-stream";
		return true;
	}
	return false;	
}

bool	RequestHandler::get_cgi_ext(std::string &ext)
{
	std::string	uri = this->_request.getUri();
	size_t		dot_pos = uri.find_last_of('.');
	static std::string exts[] = {".php", ".py"};

	if (dot_pos == std::string::npos)
		return false;
	size_t query_pos = uri.find('?', dot_pos);
	ext = (query_pos == std::string::npos) ? uri.substr(dot_pos) : uri.substr(dot_pos, query_pos - dot_pos);
	for (size_t i = 0; i < sizeof(exts) / sizeof(std::string); i++)
	{
		if (ext == exts[i])
			return true;
	}
	return false;
}
#include "Response.hpp"
#include "StatusCode.hpp"
#include "CgiHandler.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <algorithm>

Response::Response(int code, byteVector body, std::string path, bool connection)
{
	this->cursor = 0;
	std::string status = "Unknown Status";
	if (g_status_map.count(code))
        status = g_status_map[code];
	this->build_entry_line(code, status);
	this->build_header(body.size(), path, connection);
	this->_full_response.insert(this->_full_response.end(), body.begin(), body.end());
}

Response::Response(const Response &other) {
	(void)other;
}

Response	&Response::operator=(const Response &other) {
	(void)other;
	return (*this);
}

Response::~Response() {}

byteVector	&Response::get_full_response() {return this->_full_response;}

void	Response::build_entry_line(int code, std::string status)
{
	std::string		entry_line;
	std::string		method = "HTTP/1.1 ";
	std::string		delimiter = "\r\n";
	std::string		str_code;

	str_code = int_to_string(code) + ' ';
	entry_line.insert(entry_line.end(), method.begin(), method.end());
	entry_line.insert(entry_line.end(), str_code.begin(), str_code.end());
	entry_line.insert(entry_line.end(), status.begin(), status.end());
	entry_line.insert(entry_line.end(), delimiter.begin(), delimiter.end());
	this->_full_response.insert(this->_full_response.end(), entry_line.begin(), entry_line.end());
}

static std::string	get_mime_type(std::string &path)
{
	static std::map<std::string, std::string> mime_types;
	std::string			ext;
	size_t				dot_pos;

	if (mime_types.empty()) 
	{
        mime_types[".html"] = "text/html"; 
        mime_types[".css"] = "text/css";
        mime_types[".js"] = "text/javascript";
        mime_types[".png"] = "image/png";
    }
	dot_pos = path.find('.');
	if (dot_pos != std::string::npos)
	{
		ext = path.substr(dot_pos);
		if (mime_types.count(ext))
			return mime_types[ext];
	}
	return ("application/octet-stream");
}

static std::string	get_http_date()
{
	char 		buffer[100];
	time_t 		current_time = time(0);
	struct tm	*gm_time = gmtime(&current_time);

	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gm_time);
	return (std::string(buffer));
}

//TODO When another status code than 200 is send mime type may be irrevalent
void	Response::build_header(size_t body_size, std::string path, bool connection)
{
	std::string			str_size;
	std::string	buffer;

	str_size = int_to_string(body_size);
	buffer += 
	"Content-Length: " + str_size + "\r\n" +
	"Content-Type: " + get_mime_type(path) + "\r\n" +
	"Date: " + get_http_date() + "\r\n" +
	"Connection: " + (connection ? "keep-alive" : "close") +
	"\r\n\r\n";
	this->_full_response.insert(this->_full_response.end(), buffer.begin(), buffer.end());
}

#include "Sender.hpp"

Sender::Sender() {
}

Sender::Sender(const Sender &other) {
	(void) other;
}

Sender	&Sender::operator=(const Sender &other) {
	(void) other;
	return (*this);
}

Sender&	Sender::getInstance() {
	static Sender	instance;

	return instance;
}

Sender::~Sender() {
}
#include "ServerConfig.hpp"
#include <unistd.h>

ServerConfig::ServerConfig() 
: _socket(-1),
  _listenPort(0),
  _host(""),
  _serverName(""),
  _root(""),
  _index(""),
  _errorPage(""),
  _clientMaxBodySize(0),
  _defaultLocation(),
  _isDefaultSet(),
  _locations(0) {}

ServerConfig::ServerConfig(const ServerConfig &rhs)
: _socket(rhs._socket),
  _listenPort(rhs._listenPort),
  _host(rhs._host),
  _serverName(rhs._serverName),
  _root(rhs._root),
  _index(rhs._index),
  _errorPage(rhs._errorPage),
  _clientMaxBodySize(rhs._clientMaxBodySize),
  _defaultLocation(rhs._defaultLocation),
  _isDefaultSet(rhs._isDefaultSet),
  _locations(rhs._locations) {}

ServerConfig	&ServerConfig::operator=(const ServerConfig &rhs)
{
	if (this != &rhs)
	{
		this->_listenPort = rhs._listenPort;
		this->_host = rhs._host;
		this->_serverName = rhs._serverName;
		this->_root = rhs._root;
		this->_index = rhs._index;
		this->_errorPage = rhs._errorPage;
		this->_clientMaxBodySize = rhs._clientMaxBodySize;
		this->_isDefaultSet = rhs._isDefaultSet;
		this->_defaultLocation = rhs._defaultLocation;
		this->_locations = rhs._locations;
	}
	return (*this);
}

ServerConfig::~ServerConfig() 
{
	if (this->_socket > 0)
		close(this->_socket);
}

int							ServerConfig::get_socket() const {return (this->_socket);};
int							ServerConfig::get_listen_port() const {return (this->_listenPort);};
std::string					ServerConfig::get_host() const {return (this->_host);};
std::string					ServerConfig::get_server_name() const {return (this->_serverName);};
std::string					ServerConfig::get_root() const {return (this->_root);};
std::string					ServerConfig::get_index() const {return (this->_index);};
std::string					ServerConfig::get_error_page() const {return (this->_errorPage);};
size_t						ServerConfig::get_client_max_body_size() const {return (this->_clientMaxBodySize);};
LocationConfig				ServerConfig::get_default_location() const {return (this->_defaultLocation);};
bool						ServerConfig::is_default_set() const {return (this->_isDefaultSet);};
std::vector<LocationConfig>	ServerConfig::get_locations() const {return (this->_locations);};

void	ServerConfig::set_socket(int value) {this->_socket = value;};
void	ServerConfig::set_listen_port(int value) {this->_listenPort = value;};
void	ServerConfig::set_host(std::string value) {this->_host = value;};
void	ServerConfig::set_server_name(std::string value) {this->_serverName = value;};
void	ServerConfig::set_root(std::string value) {this->_root = value;};
void	ServerConfig::set_index(std::string value) {this->_index = value;};
void	ServerConfig::set_error_page(std::string value) {this->_errorPage = value;};
void	ServerConfig::set_client_mbs(size_t value) {this->_clientMaxBodySize = value;};
void	ServerConfig::set_default_location(LocationConfig value) {this->_defaultLocation = value;};
void	ServerConfig::set_is_default_set(bool value) {this->_isDefaultSet = value;};
void	ServerConfig::push_location(LocationConfig value) {this->_locations.push_back(value);};

std::ostream &operator<<(std::ostream &out, const ServerConfig &serv)
{
	out << "listen: " << serv.get_listen_port() << '\n'
	<< "server_name: " << serv.get_server_name() << '\n'
	<< "host: " << serv.get_host() << '\n'
	<< "root: " << serv.get_root() << '\n'
	<< "index: " << serv.get_index() << '\n'
	<< "error_page: " << serv.get_error_page() << '\n'
	<< "client_max_body_size: " << serv.get_client_max_body_size() << '\n'
	<< "is default set:" << serv.is_default_set() << '\n'
	<< "default location:\n" << serv.get_default_location() << '\n'
	<< "locations:\n";
	std::vector<LocationConfig>	locations = serv.get_locations();
	for (size_t i = 0; i < locations.size(); i++)
		out << locations[i];
	out << '\n';
	return (out);
}
#include "ServerHandler.hpp"
#include "Logger.hpp"
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ClientHandler.hpp"

ServerHandler::ServerHandler(WebServ& context, std::map<int, std::vector<ServerConfig> >::iterator& it) {
	int	serverFd;
	sockaddr_in	servAddr;
	_config = it->second;

	Logger::record(SETUP) << "Openning a socket...";
	if ((serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
		Logger::record(ERROR) << "Can't open socket.";
		throw AEventHandler::HandlerException("No socket avaible.");
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(it->first);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	Logger::record(SETUP) << "Binding socket.";
	if (bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
		close(serverFd);
		Logger::record(ERROR) << "ERROR, can't bind socket to port: " << it->first;
		throw AEventHandler::HandlerException("Port is not avaiable.");
	}

	Logger::record(SETUP) << "listening on port: " << it->first;
	if (listen(serverFd, SOMAXCONN) < 0) {
		close(serverFd);
		Logger::record(ERROR) << "Error, failed to listen on socket.";
		throw AEventHandler::HandlerException("Can't listen on socket.");
	}

	setSocket(serverFd);
	if (addToEpoll(context.getEpoll(), EPOLLIN) == EPOLL_CTL_FAIL)
		throw AEventHandler::HandlerException("Epoll fail");

	Logger::record(SUCCESS) << "Success, socket is ready !";
}

const std::vector<ServerConfig>&	ServerHandler::getConfig() const {return _config;}

int	ServerHandler::handleEvent(uint32_t event, WebServ& context) {
	if (event == EPOLLIN) {
		try {
			new ClientHandler(context, *this);
		}
		catch (std::exception &e) {
			Logger::err() << e.what() << std::endl;
			return SRV_FAIL_CLT;
		}
	}
	return SRV_NEW_CLT;
}

ServerHandler::~ServerHandler() {

}
#include "StatusCode.hpp"

std::map<int, std::string> g_status_map;

void init_status_map() 
{
    if (!g_status_map.empty()) 
		return;
	g_status_map[OK] = "OK";
	g_status_map[CREATED] = "Created";
	g_status_map[NO_CONTENT] = "No Content";
    g_status_map[NOT_MODIFIED] = "Not Modified";
    g_status_map[BAD_REQUEST] = "Bad Request";
    g_status_map[FORBIDDEN] = "Forbidden";
    g_status_map[NOT_FOUND] = "Not Found";
    g_status_map[METHOD_NOT_ALLOWED] = "Method Not Allowed";
    g_status_map[LENGHT_REQUIRED] = "Length Required";
    g_status_map[CONTENT_TOO_LARGE] = "Content Too Large";
    g_status_map[URI_TOO_LONG] = "URI Too Long";
    g_status_map[UNSUPORTED_MEDIA_TYPE] = "Unsupported Media Type";
    g_status_map[TOO_MANY_REQUEST] = "Too Many Requests";
    g_status_map[REQUEST_HEADER_FIELD_TOO_LARGE] = "Request Header Fields Too Large";
    g_status_map[INTERNAL_SERVER_ERROR] = "Internal Server Error";
    g_status_map[NOT_IMPLEMENTED] = "Not Implemented";
    g_status_map[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
}#include "WebServ.hpp"
#include "AEventHandler.hpp"
#include "ServerHandler.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#include <sys/signal.h>

bool	g_running = true;

void	sigHandler(int sig) {
	if (sig == SIGINT)
		g_running = false;
	Logger::record(INFO) << "Terminating WebServ...";
}

WebServ::WebServ() {

	Logger::record(SETUP) << "Creating epoll fd to manage event..."; 
	epollFd = epoll_create(1);
	if (epollFd < 0) {
		Logger::record(ERROR) << "Failed to create the epoll loop";
		throw std::runtime_error("Error, epoll failed to load...");
	}

	Logger::record(SETUP) << "Creating the singal handler...";
	signal(SIGINT, sigHandler);

	Logger::record(SUCCESS) << "You can safely quit the program with CTRL + C";
}

void	WebServ::initHost() {
	std::map<int, std::vector<ServerConfig> >	groups;
	std::vector<ServerConfig>::iterator it = serversConfig.begin();

	for (; it != serversConfig.end(); it++) {
		int port = it->get_listen_port();
		groups[port].push_back(*it);
	}

	for (std::map<int, std::vector<ServerConfig> >::iterator ite = groups.begin(); ite != groups.end(); ite++) {
		Logger::record(SETUP) << "Creating a new host to listen on port: " << ite->first;
		AEventHandler*	server;
		try {
			server = new ServerHandler(*this, ite);
		}
		catch (AEventHandler::HandlerException &e) {
			Logger::record(ERROR) << e.what();
			continue;
		}
		registery[server->getSocket()] = server;
	}
	if (registery.empty())
		throw std::runtime_error("No server is listening.\n Closing.");
}

int	WebServ::setConfig(const char* arg) {
	ConfigParser	parser;

	Logger::record(SETUP) << "Reading config file: " << arg;
	try {
		parser.parse_file(arg);
	}
	catch (ConfigException &e) {
		Logger::record(ERROR) << "Failed to load configuration file.\n"
			<< e.what();
		return CONFIG_KO;
	}
	Logger::record(SUCCESS) << "Config file loaded !";
	serversConfig = parser.get_servers();
	return CONFIG_OK;
}

void	WebServ::checkTimeout() {
	std::list<AEventHandler*>::reverse_iterator rit = timeout.rbegin();

	for (; rit != timeout.rend(); rit++) {
		AEventHandler*	curr = *rit;
		if (std::time(NULL) - curr->getTimeout() > TIMEOUT)
			removeHandler(curr);
		else
			return ;
	}
}

void	WebServ::run(const char *arg) {
	epoll_event	events[MAXEVENT];

	if (setConfig(arg) == CONFIG_KO)
		throw std::runtime_error("Temporary error may need to change it");
	initHost();

	while(g_running) {
		int nfds = epoll_wait(epollFd, events, MAXEVENT, TIMEOUT);
		for (int i = 0; i < nfds; i++) {
			AEventHandler* incoming = reinterpret_cast<AEventHandler*>(events[i].data.ptr);
			switch (incoming->handleEvent(events[i].events, *this)) {
				case CLT_MSG_ERR:
					removeHandler(incoming);
					break;
				case RM_CLT:
					removeHandler(incoming);
					break;
				case CGI_END:
					removeHandler(incoming);
					break;
				case CGI_KO:
					//TODO internal server error
					removeHandler(incoming);
					break;
				case CGI_WRITE_END:
					removeHandler(incoming);
					break;
				default:
					break ;
			}
		}
		// checkTimeout();
	}
}

void WebServ::removeHandler(AEventHandler* handler) {
	Logger::record(INFO) << "Removing handler: " << handler->getSocket();
	if (!handler) return;
	epoll_ctl(epollFd, EPOLL_CTL_DEL, handler->getSocket(), NULL);
	// timeout.erase(handler->getTimeoutIt());
	// registery.erase(handler->getSocket());
	delete handler;
}

WebServ::~WebServ() {
	std::map<int, AEventHandler*>::iterator it = registery.begin();

	for (; it != registery.end(); it++)
		delete it->second;
	if (epollFd > 0)
		close(epollFd);
}

int	WebServ::getEpoll() const {return epollFd;}
std::list<AEventHandler*>&		WebServ::getTimeList() {return timeout;}
std::map<int, AEventHandler*>&	WebServ::getRegistery() {return registery;}
