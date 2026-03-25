#include "RequestHandler.hpp"
#include "Response.hpp"
#include "StatusCode.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <sstream>
#include <sys/stat.h>

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

RequestHandler::~RequestHandler() {}

LocationConfig	&RequestHandler::getLocation() {return (this->_location);}
const ServerConfig	&RequestHandler::getServer() {return (this->_server);}

void	RequestHandler::find_corresponding_location()
{
	std::vector<LocationConfig>::iterator	it;
	std::vector<LocationConfig>	server_locations = this->_server.get_locations();
	std::vector<LocationConfig>	potential_locations;
	std::string					uri = this->_request.getUri();
	std::string					buffer;
	std::vector<std::string>	uri_blocks;
	// size_t						block_nb;
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
		if (uri.find(it->get_path()) == 0)
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

int	RequestHandler::get_cgi_loc(std::string &ext)
{
	std::vector<LocationConfig>::iterator	it;
	std::vector<LocationConfig>	server_locations = this->_server.get_locations();

	for (it = server_locations.begin(); it != server_locations.end(); it++)
	{
		if (it->get_path() == ext)
		{
			this->_location = *it;
			return 0;
		}
	}
	return 1;
}

std::string	RequestHandler::get_cgi_path()
{
	std::string	uri = this->_request.getUri();
	std::string	root;
	std::string	index;
	std::string path;

	root = this->_location.get_root().empty() ? this->_server.get_root() : this->_location.get_root();
    if (!root.empty() && root[root.size() - 1] != '/')
		root += '/';
	std::string loc_path = this->_location.get_path();
	std::string part_after_loc = uri.substr(uri.rfind("/"));
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

bool RequestHandler::is_redirection()
{
	this->find_corresponding_location();
	if (this->_location.is_redirection() == true)
		return true;
	return false;
}

Response* RequestHandler::handle_request()
{
	std::string 	method = _request.getMethod();
	std::string 	path;
	byteVector 		body;
	std::string		ext;

	this->find_corresponding_location();
	if (this->_location.is_redirection() == true)
	{
		path = this->_location.get_redirection();
		body.insert(body.end(), path.begin(), path.end());
		return new Response(MOVED_PERMANENTLY, this->_server, body, path, false);
	}
	path = this->get_file_path();
	if (method != "DELETE" && method != "POST" && method != "GET")
		return new Response(NOT_IMPLEMENTED, this->_server);
	if (method == "DELETE") 
		return this->build_delete_response();
	if (!check_if_method_allowed(this->_location, method))
		return new Response(METHOD_NOT_ALLOWED, this->_server);
	if (this->get_cgi_ext(ext))
	{
		if (this->get_cgi_loc(ext))
			return new Response(NOT_IMPLEMENTED, this->_server, byteVector(), path, true);
		return new Response(OK, this->_server, byteVector(), path, true);
	}
	if (method == "GET")
		return this->build_get_response(path);
	else if (method == "POST") 
		return new Response(CREATED, this->_server, byteVector(), path, true);
	return new Response(NOT_IMPLEMENTED, this->_server);
}

byteVector RequestHandler::get_autodindex(const std::string& path)
{
	byteVector		body;
	struct stat		fs;

	if (stat(this->_request.getUri().c_str(), &fs) == 0 && !S_ISDIR(fs.st_mode))
		return byteVector();
	if (this->_location.get_index().empty() && this->_server.get_index().empty() && this->_location.get_autoindex())
	{
		std::string response = generateAutoIndex(path, this->_request.getUri());
		body.insert(body.end(), response.begin(), response.end());
		return body;
	}
	return byteVector();
}

Response* RequestHandler::build_get_response(std::string &path)
{
	byteVector		body;
	struct stat		fs;

	body = this->get_autodindex(path);
	if (body.size() > 0)
		return new Response(OK, this->_server, body, path, true);
	int statret = stat(path.c_str(), &fs);
	if (access(path.c_str(), F_OK) != 0 || S_ISDIR(fs.st_mode))
		return new Response(NOT_FOUND, this->_server, body, path, true);
	if (statret < 0)
		return new Response(INTERNAL_SERVER_ERROR, this->_server);
	if (access(path.c_str(), R_OK) != 0)
		return new Response(FORBIDDEN, this->_server);
	return new Response(OK, this->_server, path, true);
}

Response* RequestHandler::build_delete_response()
{
	std::string path;

	this->find_corresponding_location();
	path = this->get_file_path();
	int pos = path.find_last_of('/');
	std::string dir_path = path.substr(0, pos);
	std::cout << dir_path;
	if (dir_path.empty())
		dir_path = ".";
	if (access(dir_path.c_str(), W_OK) != 0)
		return new Response(FORBIDDEN, this->_server);
	if (access(path.c_str(), F_OK) != 0)
		return new Response(NOT_FOUND, this->_server);
	if (unlink(path.c_str()) == 0)
		return new Response(NO_CONTENT, this->_server);
	else
		return new Response(INTERNAL_SERVER_ERROR, this->_server);
}

bool	RequestHandler::get_upload_type(std::string &content_type)
{
	std::string req_ct = this->_request.getHeader("Content-Type");
	if (req_ct.find("multipart/form-data") != std::string::npos) 
	{
		content_type = "multipart/form-data";
		return true;
	}
	if (req_ct.find("plain/text") != std::string::npos) 
	{
		content_type = "plain/text";
		return true;
	}
	return false;
}

bool	RequestHandler::get_cgi_ext(std::string &ext)
{
	std::string	uri = this->_request.getUri();
	size_t		q_pos = uri.find_first_of('?');
	size_t		dot_pos = uri.find_last_of('.', q_pos);
	static std::string exts[] = {".py", ".sh"};

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
