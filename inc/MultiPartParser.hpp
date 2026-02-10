#ifndef MUTLIPARTPARSER_HPP
#define MUTLIPARTPARSER_HPP

#include "HttpRequest.hpp"
#include <unistd.h>

struct	PartData {
	int fd;
	std::string			filename;
	HeaderMap			headers;
	std::vector<char>	data;

	PartData(): fd(-1) {};
	~PartData() {if (fd > 0) close(fd);}
};

class MultiPartParser {
	public:
		MultiPartParser(const std::string& boundary);
		~MultiPartParser();

		void	consume(const std::vector<char>& data, size_t len);
		void	reset();
		
		enum State{START, BOUND, HEADER, CONTENT, DONE};
	private:
		State	m_state;
		size_t	m_cursor;
		std::vector<PartData>	m_part;
		const std::string	m_bound;

		
};

#endif
