#ifndef CGICONTAINER_HPP
# define CGICONTAINER_HPP

# include "AEventHandler.hpp"

class CgiContainer : public AEventHandler 
{
	private:

	public:

		CgiContainer();
		~CgiContainer();
		CgiContainer(const CgiContainer &other);
		CgiContainer &operator=(const CgiContainer &other);

		int		handleEvent(uint32_t event, WebServ &context);
		int		get_type() const;
};

#endif
