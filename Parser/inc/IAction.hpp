#ifndef IACTION_HPP
#define IACTION_HPP

#include "IParser.hpp"

class IAction {
public:
	virtual ~IAction() {};
	virtual	void	run(Token& T) const = 0;
};

#endif
