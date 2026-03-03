#include "CgiContainer.hpp"
#include "ANetContainer.hpp"

CgiContainer::CgiContainer() : AEventHandler() {}

CgiContainer::CgiContainer(const CgiContainer &other) : AEventHandler(other) 
{
	(void)other;
}

CgiContainer	&CgiContainer::operator=(const CgiContainer &other) 
{
	(void)other;
	return (*this);
}

CgiContainer::~CgiContainer() {}

int	CgiContainer::handleEvent(uint32_t event, WebServ &context) {
	(void) event;
	(void) context;
	return 0;
}

int	CgiContainer::get_type() const 
{
	return (CGI);
}
