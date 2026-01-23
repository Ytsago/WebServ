#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ANetContainer.hpp"
# include "Respond.hpp"
# include "Request.hpp"

class Client : public ANetContainer {
	public:
		Client();										//Default constructor
		Client(std::ostream& logs, std::ostream& errLogs);
		~Client();										//Destructor
		Client(const Client &other);				//Copy constructor
		Client &operator=(const Client &other);	//Copy operator

		bool	isClient() const;
		Request&	getRequest();
		const size_t&	getIndex() const;
	
		void	setIndex(size_t newIndex);
	private:
		Request request;
		Respond	respond;

		size_t	index;
};
#endif
