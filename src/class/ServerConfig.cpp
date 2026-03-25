#include "ServerConfig.hpp"
#include <unistd.h>

ServerConfig::ServerConfig() 
: _listenPort(0),
  _host(""),
  _serverName(""),
  _root(""),
  _index(""),
  _errorPages(),
  _clientMaxBodySize(0),
  _defaultLocation(),
  _isDefaultSet(),
  _locations(0) {}

ServerConfig::ServerConfig(const ServerConfig &rhs)
: _listenPort(rhs._listenPort),
  _host(rhs._host),
  _serverName(rhs._serverName),
  _root(rhs._root),
  _index(rhs._index),
  _errorPages(rhs._errorPages),
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
		this->_errorPages = rhs._errorPages;
		this->_clientMaxBodySize = rhs._clientMaxBodySize;
		this->_isDefaultSet = rhs._isDefaultSet;
		this->_defaultLocation = rhs._defaultLocation;
		this->_locations = rhs._locations;
	}
	return (*this);
}

ServerConfig::~ServerConfig() 
{}

int							ServerConfig::get_listen_port() const {return (this->_listenPort);};
std::string					ServerConfig::get_host() const {return (this->_host);};
std::string					ServerConfig::get_server_name() const {return (this->_serverName);};
std::string					ServerConfig::get_root() const {return (this->_root);};
std::string					ServerConfig::get_index() const {return (this->_index);};
size_t						ServerConfig::get_client_max_body_size() const {return (this->_clientMaxBodySize);};
LocationConfig				ServerConfig::get_default_location() const {return (this->_defaultLocation);};
bool						ServerConfig::is_default_set() const {return (this->_isDefaultSet);};
std::vector<LocationConfig>	ServerConfig::get_locations() const {return (this->_locations);};
std::string					ServerConfig::get_error_page(int code) const 
{
	std::map<int, std::string>::const_iterator it = this->_errorPages.find(code);
	if (it != this->_errorPages.end())
		return it->second;
	return "";
};

void	ServerConfig::set_listen_port(int value) {this->_listenPort = value;};
void	ServerConfig::set_host(std::string value) {this->_host = value;};
void	ServerConfig::set_server_name(std::string value) {this->_serverName = value;};
void	ServerConfig::set_root(std::string value) {this->_root = value;};
void	ServerConfig::set_index(std::string value) {this->_index = value;};
void	ServerConfig::push_error_page(std::pair<int, std::string> value) {this->_errorPages.insert(value);};
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
	<< "error_page: " << serv.get_error_page(404) << '\n'
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
