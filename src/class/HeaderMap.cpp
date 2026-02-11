#include "HeaderMap.hpp"

HeaderMap::HeaderMap() {
}

std::string	HeaderMap::operator[](std::string key) const {
	strLower(key);

	for (size_t i = 0; i < m_headers.size(); i++) {
		if (m_headers[i].first == key)
			return m_headers[i].second;
	}
	return "";
}

HeaderMap&	HeaderMap::operator=(const HeaderMap& other) {
	if (this != &other)
		m_headers = other.m_headers;
	return *this;
}

void	HeaderMap::add(std::string key, std::string value) {
	strLower(key);
	
	for (size_t i = 0; i < m_headers.size(); i++) {
		if (m_headers[i].first == key) {
			m_headers[i].second += ", " + value;
			return ;
		}
	}
	m_headers.push_back(std::make_pair(key, value));
}

void	HeaderMap::swap(HeaderMap& other) {
	m_headers.swap(other.m_headers);
}

void	HeaderMap::reserve(int x) {
	m_headers.reserve(x);
}

void	HeaderMap::clear() {
	m_headers.clear();
}

HeaderMap::iterator	HeaderMap::begin() {
	return m_headers.begin();
}

HeaderMap::iterator	HeaderMap::end() {
	return m_headers.end();
}

HeaderMap::~HeaderMap() {

}
