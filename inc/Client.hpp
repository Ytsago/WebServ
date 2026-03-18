#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ANetContainer.hpp"
# include "HttpParser.hpp"
# include "Response.hpp"
# include "FileHandler.hpp"

# define BUFFSIZE 4096

class Client : public ANetContainer {
	public:
		Client();										//Default constructor
		Client(std::ostream& logs, std::ostream& errLogs);
		~Client();										//Destructor
		Client(const Client &other);				//Copy constructor
		Client &operator=(const Client &other);	//Copy operator

		int			get_type() const;
		const size_t&	getIndex() const;
		HttpParser&	getParser();
		Response	*getResponse();
	
		void	setIndex(size_t newIndex);
		void	setResponse(Response &response);

		void	handleRead();
		void	handleWrite();

	private:
		HttpParser	parser;
		size_t		index;
		Response	*_response;
};
#endif
