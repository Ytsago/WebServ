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

static LocationConfig	get_location(ServerConfig &server, Request &request)
{
	std::vector<LocationConfig>	server_locations = server.get_locations();
	std::string		uri = request.get_uri();
	int				location_index = 0;
	int				cur_comp;
	int				min_comp;

	min_comp = std::abs(uri.compare(server_locations[0].get_path()));
	for (size_t i = 1; i < server_locations.size(); i++)
	{
		cur_comp = std::abs(uri.compare(server_locations[i].get_path()));
		if (cur_comp < min_comp)
		{
			min_comp = cur_comp;
			location_index = i;
		}
	}
	return (server_locations[location_index]);
}

void	Response::build_response(ServerConfig &server, Request &request)
{
	//Check that host corresponds to server name
	// if not host -> bad request
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

	location = get_location(server, request);
	path = location.get_root() + location.get_index();
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
