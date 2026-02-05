#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ANetContainer.hpp"
# include "Response.hpp"
# include "Request.hpp"

class Client : public ANetContainer {
	public:
		Client();										//Default constructor
		Client(std::ostream& logs, std::ostream& errLogs);
		~Client();										//Destructor
		Client(const Client &other);				//Copy constructor
		Client &operator=(const Client &other);	//Copy operator

		int			get_type() const;
		Request&	getRequest();
		const size_t&	getIndex() const;
	
		void	setIndex(size_t newIndex);
	private:
		Request 	request;
		// Response	response;

		size_t	index;
};
#endif
