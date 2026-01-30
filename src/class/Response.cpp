#include "Response.hpp"

// static const char PAT[4] = {'\r', '\n', '\r', '\n'};

Response::Response() : AMessage() {
}

Response::Response(const Response &other) : AMessage(other) {
}

Response	&Response::operator=(const Response &other) {
	this->::AMessage::operator=(other);
	return (*this);
}

void	Response::build_entry_line(int code, std::string status)
{
	byteVector		entry_line;
	std::string		method = "HTTP/1.1 ";
	std::string		delimiter = "\r\n";
	std::string		str_code;

	str_code = int_to_string(code) + ' ';
	entry_line.insert(entry_line.end(), method.begin(), method.end());
	entry_line.insert(entry_line.end(), str_code.begin(), str_code.end());
	entry_line.insert(entry_line.end(), status.begin(), status.end());
	entry_line.insert(entry_line.end(), delimiter.begin(), delimiter.end());
	this->_entryLine = entry_line;
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

void	Response::build_header(size_t body_size, std::string path, bool connection)
{
	std::map<std::string, std::string>	header;
	std::string			str_size;

	str_size = int_to_string(body_size);
	header["Content-Length"] = str_size;
	if (!path.empty())
		header["Content-Type"] = get_mime_type(path);
	header["Date"] = get_http_date();
	if (connection)
		header["Connection"] = "keep-alive";
	else
		header["Connection"] = "close";
}

static bool check_path_correspondance(std::vector<std::string> &uri_blocks, std::vector<std::string> &loc_blocks, size_t block_nb)
{
	for (size_t i = 0; i < block_nb; i++)
	{
		if (uri_blocks[i].compare(loc_blocks[i]) != 0)
			return false;
	}
	return true;
}

static std::string	get_file_path(ServerConfig &server, Request &request)
{
	std::vector<LocationConfig>::iterator	it;
	std::vector<LocationConfig>	server_locations = server.get_locations();
	std::vector<LocationConfig>	potential_locations;
	std::string					uri = request.get_uri();
	std::string					buffer;
	std::vector<std::string>	uri_blocks;
	size_t						block_nb;
	char						del = '/';
	LocationConfig				longest_loc;
	size_t						loc_size = 0;
	size_t						longest_loc_size = 0;
	std::string					root;
	std::string					index;
	std::string 				path;

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
	longest_loc = server.get_default_location();
	for (it = potential_locations.begin(); it != potential_locations.end(); it++)
	{
		loc_size = it->get_path().size();
		if (loc_size > longest_loc_size)
		{
			longest_loc_size = loc_size;
			longest_loc = *it;
		}
	}
	root = longest_loc.get_root().empty() ? server.get_root() : longest_loc.get_root();
    if (!root.empty() && root.back() != '/')
		root += '/';
	std::string loc_path = longest_loc.get_path();
	std::string part_after_loc = uri.substr(loc_path.length());
    if (part_after_loc.empty() || part_after_loc == "/") 
    {
        index = longest_loc.get_index().empty() ? server.get_index() : longest_loc.get_index();
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

void	Response::build_response(ServerConfig &server, Request &request)
{
	if (request.get_method() == "GET")
		this->build_get_response(server, request);
	if (request.get_method() == "POST")
		this->build_post_response(server, request);
	if (request.get_method() == "DELETE")
		this->build_delete_response(server, request);
}

static LocationConfig	get_location(ServerConfig &server, Request &request)
{
	std::vector<LocationConfig>::iterator	it;
	std::vector<LocationConfig>	server_locations = server.get_locations();
	std::vector<LocationConfig>	potential_locations;
	std::string					uri = request.get_uri();
	std::string					buffer;
	std::vector<std::string>	uri_blocks;
	size_t						block_nb;
	char						del = '/';
	LocationConfig				longest_loc;
	size_t						loc_size = 0;
	size_t						longest_loc_size = 0;
	std::string					root;
	std::string					index;
	std::string 				path;

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
	longest_loc = server.get_default_location();
	return (longest_loc); 
}

void	Response::build_get_response(ServerConfig &server, Request &request)
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
	std::string	path;
	byteVector	file;
	std::string	ext;

	if (get_cgi_ext(request, ext))
	{
		//execute cgi
	}
	path = get_file_path(server, request);
	file = GetFile(path);
	this->build_entry_line(200, "OK");
	this->build_header(file.size(), path, true);
	this->_body = file;
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

static bool	get_cgi_ext(Request &request, std::string &ext)
{
	std::string	uri = request.get_uri();
	size_t		dot_pos;
	
	dot_pos = uri.find_last_of('.');
	if (dot_pos == std::string::npos)
		return false;
	else
	{
		ext = uri.substr(dot_pos);
		return true;
	}
}

static bool	get_upload_type(Request &request, std::string &content_type)
{
	std::string req_content_type = request.get_content_type();

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

void	Response::build_post_response(ServerConfig &server, Request &request)
{
	/**
	 * Get location
	 * Is POST allowed here ? 
	 * 		-> No 405
	 * Is this a CGI script? (Path/Extension check) 
	 * 		-> Yes: Execute script → Pipe any body content to stdin → Return script's output.
	 * Is this a "Static" Upload? (Check Content-Type)
	 * 		-> Is it multipart/form-data? → Use your internal upload logic.
	 *		-> Is it application/octet-stream? → Use your internal upload logic.
	 * None of the above ?
	 * 		-> 415
	 */
	LocationConfig	location;
	std::string		ext;
	std::string		content_type;

	location = get_location(server, request);
	
	if (!check_if_method_allowed(location, "POST"))
	{
		this->build_entry_line(405, "Method Not Allowed");
		this->build_header(0, "", true);
	}
	if (get_cgi_ext(request, ext))
	{
		//execute cgi
	}
	if (get_upload_type(request, content_type))
	{
		//upload file
	}
	else
	{
		this->build_entry_line(415, "Unsupported Media Type");
		this->build_header(0, "", true);
	}
}

void	Response::build_delete_response(ServerConfig &server, Request &request)
{
	(void)server;
	(void)request;
}

Response::~Response() {
}
