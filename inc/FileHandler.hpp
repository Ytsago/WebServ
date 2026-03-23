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

		FileHandler();
		FileHandler(HttpRequest &request, const ServerConfig *server, std::string &content_type, std::string content_length);
		FileHandler&	operator=(const FileHandler& other);
		~FileHandler();
		
		void			multiparse(const std::vector<char> &chunk);
		void			handle_plaintext(const std::vector<char> &chunk);
		std::string		create_filename();
		int				getState();

	private:
	
		const ServerConfig*		_server;
		size_t				_bodySize;
		size_t				_contentLength;
		std::string			_boundary;
		std::string			_filename;
		std::string			_uploadPath;
		std::vector<char>	_buffer;
		MultipartState		_state;
		int					_fileFd;
};

#endif
