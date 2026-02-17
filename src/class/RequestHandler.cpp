#include "RequestHandler.hpp"
#include "Response.hpp"
#include "CgiHandler.hpp"
#include "StatusCode.hpp"
#include "FileHandler.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <algorithm>
#include <map>

RequestHandler::RequestHandler(ServerConfig &server, HttpRequest &request, int &epollFd) :
	_server(server),
	_request(request),
	_location(),
	_epollFd(epollFd) {}

RequestHandler::RequestHandler(const RequestHandler &other) :
	_server(other._server),
	_request(other._request),
	_location(other._location),
	_epollFd(other._epollFd) {}

RequestHandler	&RequestHandler::operator=(const RequestHandler &other) 
{
	if (this != &other)
	{
		this->_server = other._server;
		this->_request = other._request;
		this->_location = other._location;
		this->_epollFd = other._epollFd;
	}
	return (*this);
}

RequestHandler::~RequestHandler() {}

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

bool	RequestHandler::get_cgi_ext(std::string &ext)
{
	std::string	uri = this->_request.getUri();
	size_t		dot_pos;
	size_t		query_pos;
	
	dot_pos = uri.find_last_of('.');
	if (dot_pos == std::string::npos)
		return false;
	else
	{
		query_pos = uri.find('?', dot_pos);
		ext = (query_pos == std::string::npos) ? uri.substr(dot_pos) : uri.substr(dot_pos, query_pos - dot_pos);
		return true;
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

Response	*RequestHandler::build_get_response()
{
	/**
	 * Find corresponding location
	 * Build header: 
	 * 		entry-line: "HTTP/1.1" + " " + "CODE" + " " + "STATUS" + "\r\n"
	 * 		map:
	 * 			Content-Length: body.size()
	 * 			Content-Type: check file extension and find corresponding MIME type (create mime type static map) 
	 * 			Date: timestamp RFC 7231 format
	 * 			Etag ?
	 * 			Connection: "keep-alive" or "close"
	 * 	Body: data
	**/
	std::string		path;
	byteVector		file;
	std::string		ext;

	this->find_corresponding_location();
	path = this->get_file_path();
	if (get_cgi_ext(ext))
		CgiHandler::execute_cgi(this->_server, this->_request, this->_location, path, this->_epollFd);
	file = GetFile(path);
	Response *response = new Response(OK, file, path, true);
	return (response);
}

static bool	check_if_method_allowed(LocationConfig &location, std::string method)
{
	std::vector<std::string> 	allowed_methods = location.get_methods();

	for (size_t i = 0; i < allowed_methods.size(); i++)
	{
		if (allowed_methods[i].compare(method) == 0)
			return true;
	}
	return false;
}

bool	RequestHandler::get_upload_type(std::string &content_type)
{
	std::string req_content_type = this->_request.getHeader("Content-Type");

	if (req_content_type.compare("multipart/form-data") == 0)
	{
		content_type = "multipart/form-data";
		return true;
	}
	if (req_content_type.compare("application/octet-stream") == 0)
	{
		content_type = "application/octet-stream";
		return true;
	}
	return false;
}

Response	*RequestHandler::build_post_response()
{
	/**
	 * Get location
	 * Is POST allowed here ? 
	 * 		-> No 405
	 * Is this a CGI script? (Path/Extension check) 
	 * 		-> Yes: Execute script -> Pipe any body content to stdin -> Return script's output.
	 * Is this a "Static" Upload? (Check Content-Type)
	 * 		-> Is it multipart/form-data? -> Use internal upload logic.
	 *		-> Is it application/octet-stream? -> Use internal upload logic.
	 * None of the above ?
	 * 		-> 415 
	 */
	std::string		ext;
	std::string		path;
	std::string		content_type;
	Response 		*response;

	this->find_corresponding_location();
	path = this->get_file_path();
	if (!check_if_method_allowed(this->_location, "POST"))
	{
		response = new Response(METHOD_NOT_ALLOWED);
	}
	if (this->get_cgi_ext(ext))
	{
		CgiHandler::execute_cgi(this->_server, this->_request, this->_location, path, this->_epollFd);
		//Response
	}
	if (this->get_upload_type(content_type))
	{
		FileHandler file_handler(this->_request, this->_location, content_type);
		//Response
	}
	else
	{
		response = new Response(UNSUPORTED_MEDIA_TYPE);
	}
	return (response);
}

Response	*RequestHandler::build_delete_response()
{
	Response 		*response = new Response(UNSUPORTED_MEDIA_TYPE);
	return (response);
}

Response	*RequestHandler::handle_request()
{
	if (this->_request.getMethod() == "GET")
		return this->build_get_response();
	if (this->_request.getMethod() == "POST")
		return this->build_post_response();
	if (this->_request.getMethod() == "DELETE")
		return this->build_delete_response();
	return NULL;
}
