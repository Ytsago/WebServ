#include "FileHandler.hpp"
#include "HttpParser.hpp"
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

FileHandler::FileHandler() : _fileFd(-1) {}

FileHandler::FileHandler(HttpRequest &request, const ServerConfig *server, std::string &content_type, std::string content_length) :
	_server(server),
	_bodySize(0),
	_contentLength(0),
	_state(SEARCH_BOUNDARY),
	_fileFd(-1)
{
	this->_uploadPath = "./" + _server->get_root() + "uploads/";
	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		std::string h_content = request.getHeader("Content-Type");
		size_t pos = h_content.find("boundary=");
		if (pos != std::string::npos)
			this->_boundary = "--" + h_content.substr(pos + 9);
	}
	if (content_type.find("text/plain") != std::string::npos)
	{
		char* endptr;
		this->_contentLength = std::strtol(content_length.c_str(), &endptr, 10);
		if (*endptr != '\0')
		{
			std::cout << "111111\n";
			throw (HttpParser::HttpRequestParsingException(INTERNAL_SERVER_ERROR));
		}
		this->_filename = this->create_filename();
		this->_filename = this->_uploadPath + this->_filename;
		this->_fileFd = open(this->_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (this->_fileFd < 0)
			std::cout << "3333\n";
		this->_state = WRITING_DATA;
	}

}

FileHandler&	FileHandler::operator=(const FileHandler& other) {
	if (this != &other) {
		_state = other._state;
		_contentLength = other._contentLength;
		_fileFd = other._fileFd;
		_boundary = other._boundary;
		_buffer = other._buffer;
		_filename = other._filename;
		_uploadPath = other._uploadPath;
		_server = other._server;
		_bodySize = other._bodySize;
	}
	return (*this);
}

FileHandler::~FileHandler() 
{
	if (_fileFd != -1)
		close(_fileFd);
}

int	FileHandler::getState() { return (this->_state); }

static std::string sanitize_filename(std::string filename)
{
	std::stringstream	ss;
	std::string 		sanitized;
	size_t				last_slash = filename.find_last_of("/");

	sanitized = (last_slash == std::string::npos) ? filename : filename.substr(last_slash + 1);
	if (sanitized.empty())
		sanitized = "default";
	ss << time(NULL) << "_" << sanitized;
	sanitized = ss.str();
	return (sanitized);
}

static std::string extract_filename(std::vector<char>::iterator begin, std::vector<char>::iterator end)
{
	std::string filename;
	std::string header(begin, end);
	size_t 		pos = header.find("filename=\"");

	if (pos == std::string::npos)
		return sanitize_filename("unknown_file");
	size_t first = pos + 10;
	size_t last = header.find("\"", first);
	filename = header.substr(first, last - first);
	return (sanitize_filename(filename));
}

std::string FileHandler::create_filename()
{
	std::string filename = "plaintext_file";
	
	return (sanitize_filename(filename));
}

void FileHandler::multiparse(const std::vector<char> &chunk) 
{
	this->_buffer.insert(this->_buffer.end(), chunk.begin(), chunk.end());
	_bodySize += _buffer.size();
	if (_bodySize >= _server->get_client_max_body_size())
		throw (HttpParser::HttpRequestParsingException(CONTENT_TOO_LARGE));
	while (true) 
	{
		if (this->_state == SEARCH_BOUNDARY) 
		{
			std::vector<char>::iterator it = std::search(this->_buffer.begin(), this->_buffer.end(), this->_boundary.begin(), this->_boundary.end());
			if (it == this->_buffer.end()) 
				break;
			this->_buffer.erase(this->_buffer.begin(), it + this->_boundary.size());
			this->_state = PARSE_HEADERS;
		}
		if (this->_state == PARSE_HEADERS) 
		{
			const char *del = "\r\n\r\n";
			std::vector<char>::iterator it = std::search(this->_buffer.begin(), this->_buffer.end(), del, del + 4);
			if (it == this->_buffer.end()) 
				break;
			this->_filename = extract_filename(this->_buffer.begin(), it);
			this->_filename = this->_uploadPath + this->_filename;
			this->_fileFd = open(this->_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			this->_buffer.erase(this->_buffer.begin(), it + 4);
			this->_state = WRITING_DATA;
		}
		if (this->_state == WRITING_DATA)
		{
			const char *end_boundary = "--";
			std::vector<char>::iterator it = std::search(this->_buffer.begin(),this-> _buffer.end(), this->_boundary.begin(), this->_boundary.end());
			if (it != _buffer.end())
			{
				size_t boundary_offset = std::distance(this->_buffer.begin(), it);
				if (boundary_offset >= 2)
				{
					if (write(this->_fileFd, &(*this->_buffer.begin()), boundary_offset - 2) < 1)
						throw (HttpParser::HttpRequestParsingException(INTERNAL_SERVER_ERROR));
				}
				close(this->_fileFd);
				bool is_end = false;
				if (std::distance(it + this->_boundary.size(), this->_buffer.end()) >= 2)
				{
					if (std::equal(end_boundary, end_boundary + 2, it + this->_boundary.size()))
						is_end = true;
				}
				else if (std::distance(it + this->_boundary.size(), this->_buffer.end()) < 2)
					break;
				this->_buffer.erase(this->_buffer.begin(), it);
				if (is_end) 
				{
					this->_state = END;
					this->_buffer.clear();
				} 
				else
					this->_state = SEARCH_BOUNDARY;
			}
			else 
			{
				if (this->_buffer.size() > this->_boundary.size() + 4) 
				{
					size_t write_size = this->_buffer.size() - (this->_boundary.size() + 4);
					if (write(this->_fileFd, &(*this->_buffer.begin()), write_size) < 1)
						throw (HttpParser::HttpRequestParsingException(INTERNAL_SERVER_ERROR));
					this->_buffer.erase(this->_buffer.begin(), this->_buffer.begin() + write_size);
				}
				break; 
			}
		}
		if (this->_state == END)
			break;
	}
}

void FileHandler::handle_plaintext(const std::vector<char> &chunk) 
{
	this->_buffer.insert(this->_buffer.end(), chunk.begin(), chunk.end());
	_bodySize += chunk.size();
	if (_bodySize >= _server->get_client_max_body_size())
		throw (HttpParser::HttpRequestParsingException(CONTENT_TOO_LARGE));
	if (!this->_buffer.empty())
	{
		ssize_t bytes_written = write(this->_fileFd, &(*this->_buffer.begin()), this->_buffer.size());
		if (bytes_written < 0)
			throw (HttpParser::HttpRequestParsingException(INTERNAL_SERVER_ERROR));
		this->_buffer.erase(this->_buffer.begin(), this->_buffer.begin() + bytes_written);
	}
	if (this->_bodySize >= this->_contentLength)
	{
		close(this->_fileFd);
		this->_state = END;
	}
}
