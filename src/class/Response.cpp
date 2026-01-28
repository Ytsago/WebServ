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

void	Response::build_header(size_t body_size, std::string &path, bool connection)
{
	std::map<std::string, std::string>	header;
	std::string			str_size;

	str_size = int_to_string(body_size);
	header["Content-Length"] = str_size;
	header["Content-Type"] = get_mime_type(path);
	header["Date"] = get_http_date();
	if (connection)
		header["Connection"] = "keep-alive";
	else
		header["Connection"] = "close";
}

// static std::string get_path_block(std::string &path, size_t block_nb)
// {
// 	std::string	block;
// 	size_t		pos;
// 	size_t		prev_pos = 0;
// 	size_t		cur_block = 0;
// 	bool		last_block = false;

// 	while (!last_block) 
// 	{
// 		pos = path.find('/', prev_pos);
// 		if (pos == std::string::npos) 
// 		{
// 			pos = path.length();
// 			last_block = true;
// 		}
// 		if (cur_block < block_nb)
// 		{
// 			prev_pos = pos;
// 			cur_block++;
// 			continue;
// 		}
// 		if (pos == std::string::npos) 
// 		{
// 			pos = path.length();
// 			last_block = true;
// 		}
// 		block = path.substr(prev_pos, pos - prev_pos);
// 		return (block);
// 	}
// }

// static std::string	get_location_path(ServerConfig &server, Request &request)
// {
// 	std::vector<LocationConfig>::iterator	it;
// 	std::vector<LocationConfig>	server_locations = server.get_locations();
// 	std::vector<LocationConfig>	potential_locations;
// 	std::string					uri = request.get_uri();
// 	LocationConfig				longest_loc;
// 	std::string					uri_block;
// 	std::string					loc_block;
// 	std::string					root;
// 	std::string					index;
// 	std::string 				path;
// 	size_t						pos;
// 	size_t						prev_pos = 0;
// 	size_t						cur_block = 1;
// 	size_t						loc_size = 0;
// 	size_t						longest_loc_size = 0;

// 	//uri = "/yo/yes/haha"
// 	pos = uri.find('/');
// 	for (size_t i = 0; i < server_locations.size(); i++)
// 	{
// 		if (uri.compare(0, 1, server_locations[i].get_path()) == 0 && server_locations[i].is_cgi() == false)
// 			potential_locations.push_back(server_locations[i]);
// 	}
// 	prev_pos = pos;
// 	while (true)
// 	{
// 		uri_block = get_path_block(uri, cur_block);
// 		if (uri_block.empty())
// 			break;
// 		for (it = potential_locations.begin(); it != potential_locations.end(); it++)
// 		{
// 			std::string loc_path = it->get_path();
// 			loc_block = get_path_block(loc_path, cur_block);
// 			if (loc_block.empty())
// 				continue;
// 			if (uri_block.compare(loc_block) != 0)
// 				it = potential_locations.erase(it);
// 		}
// 		cur_block++;
// 	}

// 	for (it = potential_locations.begin(); it != potential_locations.end(); it++)
// 	{
// 		loc_size = it->get_path().size();
// 		if (loc_size > longest_loc_size)
// 		{
// 			longest_loc_size = loc_size;
// 			longest_loc = *it;
// 		}
// 	}
// 	root = longest_loc.get_root();
// 	if (root.empty())
// 		root = server.get_root();
// 	index = longest_loc.get_index();
// 	if (index.empty())
// 		index = server.get_index();
// 	path = root + index;
// 	return (path); 
// }

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
	std::string		path;
	LocationConfig	location;
	byteVector		file;

	//Is_cgi ? -> launch cgi
	path = get_file_path(server, request);
	file = GetFile(path);
	this->build_entry_line(200, "OK");
	this->build_header(file.size(), path, true);
	this->_body = file;
}

void	Response::build_post_response(ServerConfig &server, Request &request)
{
	(void)server;
	(void)request;
}

void	Response::build_delete_response(ServerConfig &server, Request &request)
{
	(void)server;
	(void)request;
}

// void	Response::build_response(ServerConfig &server, Request &request) {
// 	byteVector	msg(_entryLine);

// 	msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	for (std::map<std::string, std::string>::const_iterator it = _headerField.begin(); it != _headerField.end(); it++) {
// 		msg.insert(msg.end(), it->first.begin(), it->first.end() -1);
// 		msg.push_back(':');
// 		msg.insert(msg.end(), it->second.begin(), it->second.end() -1);
// 		msg.insert(msg.end(), PAT, PAT + 4);
// 	}
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	if (this->checkFlag(BODY))
// 		msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	setRaw(msg);
// }

Response::~Response() {
}
