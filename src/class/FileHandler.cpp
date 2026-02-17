	#include "FileHandler.hpp"
	#include "ANetContainer.hpp"
	#include <algorithm>
	#include "unistd.h"
	#include <fcntl.h>

	FileHandler::FileHandler(HttpRequest &request, LocationConfig &location, std::string &content_type) : _request(request), _location(location), _contentType(content_type) , _state(SEARCH_BOUNDARY)
	{
		if (content_type == "multipart/form-data")
		{
			std::string content_type = request.getHeader("Content-Type");
			size_t pos = content_type.find("boundary=");
			this->_boundary = "--" + content_type.substr(pos + 9);
			this->multiparse(request.getBody());
		}
	}

	// FileHandler::FileHandler(const FileHandler &other) : _request(other._request), _location(other._location), _contentType(other._contentType), _boundary(other._boundary), _state(other._state) {}

	// FileHandler	&FileHandler::operator=(const FileHandler &other) 
	// {
	// 	if (this != &other)
	// 	{
	// 		this->_request = other._request;
	// 		this->_location = other._location;
	// 		this->_contentType = other._contentType;
	// 		this->_boundary = other._boundary;
	// 		this->_state = other._state;
	// 	}
	// 	return (*this);
	// }

	FileHandler::~FileHandler() {}

	static std::string sanitize_filename(std::string &filename)
	{
		std::stringstream	ss;
		std::string 		sanitized;
		size_t				last_slash = filename.find_last_of("/");

		if (last_slash != std::string::npos)
			sanitized = filename.substr(last_slash + 1);
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
		{
			//Exception
		}
		size_t first = pos + 10;
		size_t last = header.find("\"", first);
		filename = header.substr(first, last - first);
		return (sanitize_filename(filename));
	}

	void FileHandler::multiparse(const std::vector<char> &chunk) 
	{
		this->_buffer.insert(this->_buffer.end(), chunk.begin(), chunk.end());
		while (true) 
		{
			if (this->_state == SEARCH_BOUNDARY) 
			{
				std::vector<char>::iterator it = std::search(this->_buffer.begin(), this->_buffer.end(), this->_boundary.begin(), this->_boundary.end());
				if (it == this->_buffer.end()) 
					break;
				size_t pos = std::distance(this->_buffer.begin(), it) + this->_boundary.size();
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
				this->_fileFd = open(this->_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
				this-> _buffer.erase(this->_buffer.begin(), it + 4);
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
						write(this->_fileFd, &(*this->_buffer.begin()), boundary_offset - 2);
					close(this->_fileFd);
					bool is_end = false;
					if (std::distance(it + this->_boundary.size(), this->_buffer.end()) >= 2)
					{
						if (std::equal(end_boundary, end_boundary + 2, it + this->_boundary.size()))
							is_end = true;
					}
					else if (std::distance(it + this->_boundary.size(), this->_buffer.end()) < 2)
						break; 
					this->_buffer.erase(this->_buffer.begin(), it + this->_boundary.size());
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
						write(this->_fileFd, &(*this->_buffer.begin()), write_size);
						this->_buffer.erase(this->_buffer.begin(), this->_buffer.begin() + write_size);
					}
					break; 
				}
			}
		}
	}