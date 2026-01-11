#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>

class Request {
	public:
		Request();										//Default constructor
		~Request();										//Destructor
		Request(const Request &other);				//Copy constructor
		Request &operator=(const Request &other);	//Copy operator
	private:

};
#endif
