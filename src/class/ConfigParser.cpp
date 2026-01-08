#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : _servConf(), _locConf() {}

ConfigParser::ConfigParser(const char *arg) : _servConf(), _locConf()
{
	this->parse_input(arg);
	this->print_conf();
}

ConfigParser::~ConfigParser() {}

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

void	ConfigParser::parse_input(const char *arg)
{
	std::ifstream	inFile;
	std::string		line;

	inFile.open(arg);
	if (!inFile.is_open())
	{
		//Exception
	}
	while (getline(inFile, line))
	{
		trim(line);
		if (line.empty() || line[0] == '#') 
			continue;
		if (line == "server {")
			this->parse_server(inFile);
	}
	inFile.close();
}

void	ConfigParser::parse_server(std::ifstream &file)
{
	std::string line;
	std::string key;
	std::string s_value;
	int			i_value;
	size_t		ui_value;

	while (std::getline(file, line)) 
	{
		trim(line);
		if (line == "}")
			break ;
		if (line.empty() || line[0] == '#')
			continue ;
		std::stringstream ss(line);
		ss >> key;
		if (key == "listen")
		{
			ss >> i_value;
			this->_servConf.set_listen_port(i_value);
		}
		else if (key == "server_name")
		{
			ss >> s_value;
			this->_servConf.set_server_name(s_value);
		}
		else if (key == "host")
		{
			ss >> s_value;
			this->_servConf.set_host(s_value);
		}
		else if (key == "root")
		{
			ss >> s_value;
			this->_servConf.set_root(s_value);
		}
		else if (key == "index")
		{
			ss >> s_value;
			this->_servConf.set_index(s_value);
		}
		else if (key == "error_page")
		{
			ss >> s_value;
			this->_servConf.set_error_page(s_value);
		}
		else if (key == "client_max_body_size")
		{
			ss >> ui_value;
			this->_servConf.set_client_mbs(ui_value);
		}
	}
}

void	ConfigParser::print_conf() const
{
	this->_servConf.print();
}
