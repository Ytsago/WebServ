#include "LocationConfig.hpp"

LocationConfig::LocationConfig() 
: _path(""),
  _root(""),
  _index(""),
  _methods(0),
  _autoindex(0),
  _cgiExt(""),
  _cgiPath(""),
  _isCgi(0),
  _isRedirection(false),
  _redirection("") {}

LocationConfig::LocationConfig(const LocationConfig &rhs)
: _path(rhs._path),
  _root(rhs._root),
  _index(rhs._index),
  _methods(rhs._methods),
  _autoindex(rhs._autoindex),
  _cgiExt(rhs._cgiExt),
  _cgiPath(rhs._cgiPath),
  _isCgi(rhs._isCgi),
  _isRedirection(rhs._isRedirection),
  _redirection(rhs._redirection) {}

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
		this->_isRedirection = rhs._isRedirection;
		this->_redirection = rhs._cgiPath;
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
bool						LocationConfig::is_redirection() const {return (this->_isRedirection);};
std::string					LocationConfig::get_redirection() const {return (this->_redirection);};

void	LocationConfig::set_path(std::string value) {this->_path = value;};
void	LocationConfig::set_root(std::string value) {this->_root = value;};
void	LocationConfig::set_index(std::string value) {this->_index = value;};
void	LocationConfig::push_method(std::string value) {this->_methods.push_back(value);};
void	LocationConfig::set_autoindex(bool value) {this->_autoindex = value;};
void	LocationConfig::set_cgi_ext(std::string value) {this->_cgiExt = value;};
void	LocationConfig::set_cgi_path(std::string value) {this->_cgiPath = value;};
void	LocationConfig::set_is_cgi(bool value) {this->_isCgi = value;};
void	LocationConfig::set_is_redirection(bool value) {this->_isRedirection = value;};
void	LocationConfig::set_redirection(std::string value) {this->_redirection = value;}; 

std::ostream &operator<<(std::ostream &out, const LocationConfig &location)
{
	out << "location: " << '\n' 
	<< "\tis_cgi: " << (location.is_cgi() ? "true" : "false") << '\n'
	<< "\tis_redirection: " << (location.is_redirection() ? "true" : "false") << '\n'
	<< "\tredirection: " << location.get_redirection() << '\n'
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
}