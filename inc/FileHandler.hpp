#ifndef FILEHANDLER_HPP
# define FILEHANDLER_HPP

#include "HttpRequest.hpp"
#include "LocationConfig.hpp"

class FileHandler
{
	public:

		FileHandler(HttpRequest &request, LocationConfig &location, std::string &content_type);
		~FileHandler();
		FileHandler(const FileHandler &other);
		FileHandler &operator=(const FileHandler &other);

		int		getState();

		void	multiparse(const std::vector<char> &chunk);

			enum	MultipartState 
			{
				SEARCH_BOUNDARY, 
				PARSE_HEADERS, 
				WRITING_DATA,
				END
			};

		private:

			HttpRequest			&_request;
			LocationConfig		&_location;
			std::string			&_contentType;
			std::string			_boundary;
			std::string			_filename;
			std::string			_uploadPath;
			std::vector<char>	_buffer;
			MultipartState		_state;
			int					_fileFd;
};
#endif