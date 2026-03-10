#include "Response.hpp"
#include "StatusCode.hpp"
#include "CgiHandler.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <algorithm>

Response::Response(int code, byteVector body, std::string path, bool connection)
{
	this->cursor = 0;
	std::string status = "Unknown Status";
	if (g_status_map.count(code))
        status = g_status_map[code];
	this->build_entry_line(code, status);
	this->build_header(body.size(), path, connection);
	this->_full_response.insert(this->_full_response.end(), body.begin(), body.end());
}

Response::Response(const Response &other) {
	(void)other;
}

Response	&Response::operator=(const Response &other) {
	(void)other;
	return (*this);
}

Response::~Response() {}

byteVector	&Response::get_full_response() {return this->_full_response;}

void	Response::build_entry_line(int code, std::string status)
{
	std::string		entry_line;
	std::string		method = "HTTP/1.1 ";
	std::string		delimiter = "\r\n";
	std::string		str_code;

	str_code = int_to_string(code) + ' ';
	entry_line.insert(entry_line.end(), method.begin(), method.end());
	entry_line.insert(entry_line.end(), str_code.begin(), str_code.end());
	entry_line.insert(entry_line.end(), status.begin(), status.end());
	entry_line.insert(entry_line.end(), delimiter.begin(), delimiter.end());
	this->_full_response.insert(this->_full_response.end(), entry_line.begin(), entry_line.end());
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
	std::string			str_size;
	std::string	buffer;

	str_size = int_to_string(body_size);
	buffer += 
	"Content-Length: " + str_size + "\r\n" +
	"Content-Type: " + get_mime_type(path) + "\r\n" +
	"Date: " + get_http_date() + "\r\n" +
	"Connection: " + (connection ? "keep-alive" : "close") +
	"\r\n\r\n";
	this->_full_response.insert(this->_full_response.end(), buffer.begin(), buffer.end());
}

