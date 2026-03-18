#ifndef PARSERMANAGER_HPP
#define PARSERMANAGER_HPP

#include "IParser.hpp"
#include <vector>

class ParserManager {
	std::vector<IParser*>	_parsers;
public:
	~ParserManager() {
		for (size_t i = 0; i < _parsers.size(); i++) delete _parsers[i];
	}

	template <typename T>
	T&	track(T* parser) {
		_parsers.push_back(parser);
		return *parser;
	}
};

#endif
