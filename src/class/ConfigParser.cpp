#include "ConfigParser.hpp"
#include "Logger.hpp"
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
	location.set_autoindex(false);
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
		else if (key == "return")
		{
			if (!(ss_line >> s_value) || s_value != "301")
				throw ConfigException("Invalid redirection value", this->_lineCount);
			if (!(ss_line >> s_value))
				throw ConfigException("Invalid redirection value", this->_lineCount);
			location.set_redirection(s_value);
			location.set_is_redirection(true);
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
