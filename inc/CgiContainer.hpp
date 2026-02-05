#ifndef CGICONTAINER_HPP
# define CGICONTAINER_HPP

# include "ANetContainer.hpp"

class CgiContainer : public ANetContainer 
{
	private:

	public:

		CgiContainer();
		~CgiContainer();
		CgiContainer(const CgiContainer &other);
		CgiContainer &operator=(const CgiContainer &other);

		int		get_type() const;
};

#endif