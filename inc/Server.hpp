#ifndef SERVER_HPP
# define SERVER_HPP

# include "ANetContainer.hpp"

class Server : public ANetContainer {
	public:
		Server();										//Default constructor
		Server(std::ostream& logs, std::ostream& errLogs);
		~Server();										//Destructor
		Server(const Server &other);				//Copy constructor
		Server &operator=(const Server &other);	//Copy operator
	
		bool	isClient() const;
	private:

};
#endif
