#include "MultiPartParser.hpp"

MultiPartParser::MultiPartParser(const std::string& boundary) : m_state(START), m_cursor(0), m_bound(boundary) {
}

void	MultiPartParser::consume(const std::vector<char>& data, size_t len) {
	bool	again = true;

	while (again && m_cursor < len) {
		switch (m_state) {
			case START: again = true;
			case BOUND: {
				
			}
			case HEADER:
			case CONTENT:
			case DONE:
		}
	}
}

MultiPartParser::~MultiPartParser() {

}
