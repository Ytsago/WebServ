#include "ServerConfig.hpp"

ServerConfig::ServerConfig() 
: _listen_port(0),
  _host(""),
  _server_name(""),
  _root(""),
  _index(""),
  _error_page(""),
  _client_max_body_size(0) {}

ServerConfig::~ServerConfig() {}

void	ServerConfig::set_listen_port(int value) {this->_listen_port = value;};
void	ServerConfig::set_host(std::string value) {this->_host = value;};
void	ServerConfig::set_server_name(std::string value) {this->_server_name = value;};
void	ServerConfig::set_root(std::string value) {this->_root = value;};
void	ServerConfig::set_index(std::string value) {this->_index = value;};
void	ServerConfig::set_error_page(std::string value) {this->_error_page = value;};
void	ServerConfig::set_client_mbs(size_t value) {this->_client_max_body_size = value;};

void	ServerConfig::print() const
{
	std::cout << "listen: " << this->_listen_port << '\n';
	std::cout << "server_name: " << this->_server_name << '\n';
	std::cout << "host: " << this->_host << '\n';
	std::cout << "root: " << this->_root << '\n';
	std::cout << "index: " << this->_index << '\n';
	std::cout << "error_page: " << this->_error_page << '\n';
	std::cout << "client_max_body_size: " << this->_client_max_body_size << '\n';
}
