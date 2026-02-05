#ifndef ANETCONTAINER_HPP
# define ANETCONTAINER_HPP

#include <iostream>

enum e_container_type
{
	CLIENT,
	SERVER,
	CGI
};

class ANetContainer {
	public:
		ANetContainer();										//Default constructor
		ANetContainer(std::ostream& logs, std::ostream& errLogs);
		virtual ~ANetContainer();										//Destructor
		ANetContainer(const ANetContainer &other);				//Copy constructor
		ANetContainer &operator=(const ANetContainer &other);	//Copy operator
	
		virtual int	get_type() const = 0;

		int		getSocket() const;
		void	setSocket(const int& socket);
	private:
		int	socket;
		std::ostream& logs;
		std::ostream& errLogs;
};
#endif
