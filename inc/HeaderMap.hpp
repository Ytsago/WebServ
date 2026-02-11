#ifndef HEADERMAP_HPP
#define HEADERMAP_HPP

#include <vector>
#include <string>
#include "utils.hpp"

class HeaderMap {
	public:
		typedef std::vector<std::pair<std::string, std::string> >::iterator iterator;

		HeaderMap();
		~HeaderMap();

		void	add(std::string key, std::string value);
		void	swap(HeaderMap& other);
		void	reserve(int x);
		void	clear();

		std::string	operator[](std::string key) const;
		HeaderMap&	operator=(const HeaderMap& other);

		iterator begin();
		iterator end();
	private:
		std::vector<std::pair<std::string, std::string> > m_headers;
};

#endif
