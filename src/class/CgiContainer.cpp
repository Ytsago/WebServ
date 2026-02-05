#include "CgiContainer.hpp"
#include "ANetContainer.hpp"

CgiContainer::CgiContainer() : ANetContainer() {}

CgiContainer::CgiContainer(const CgiContainer &other) : ANetContainer(other) 
{
	(void)other;
}

CgiContainer	&CgiContainer::operator=(const CgiContainer &other) 
{
	(void)other;
	return (*this);
}

CgiContainer::~CgiContainer() {}

int	CgiContainer::get_type() const 
{
	return (CGI);
}
