#ifndef FILEHANDLER_HPP
# define FILEHANDLER_HPP

# include <vector>
# include <string>
# include <sstream>
# include "HttpRequest.hpp"
# include "LocationConfig.hpp"

class FileHandler
{
	public:

		enum MultipartState 
		{
			SEARCH_BOUNDARY, 
			PARSE_HEADERS, 
			WRITING_DATA,
			END
		};

		FileHandler(HttpRequest &request, LocationConfig &location, std::string &content_type);
		~FileHandler();
		
		void			multiparse(const std::vector<char> &chunk);
		int				getState();

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