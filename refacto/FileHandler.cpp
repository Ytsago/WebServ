#include "FileHandler.hpp"
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

FileHandler::FileHandler(HttpRequest &request, LocationConfig &location, std::string &content_type) :
	_request(request),
	_location(location),
	_contentType(content_type),
	_state(SEARCH_BOUNDARY),
	_fileFd(-1)
{
	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		std::string h_content = request.getHeader("Content-Type");
		size_t pos = h_content.find("boundary=");
		if (pos != std::string::npos)
			this->_boundary = "--" + h_content.substr(pos + 9);
	}
	// Le chemin d'upload devrait idéalement venir de la configuration de la Location
	this->_uploadPath = "./website/uploads/"; 
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

// static void	print_buf(std::vector<char> &buf)
// {
// 	for (std::vector<char>::iterator ite = buf.begin(); ite != buf.end(); ite++)
// 		std::cout << *ite;
// 	std::cout << "+++++++++++++++++++++\n";
// }

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
			// std::cout << this->_filename << '\n';
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
					write(this->_fileFd, &(*this->_buffer.begin()), write_size);
					this->_buffer.erase(this->_buffer.begin(), this->_buffer.begin() + write_size);
				}
				break; 
			}
		}
		if (this->_state == END)
			break;
	}
}