#ifndef RESPOND_HPP
# define RESPOND_HPP

#include <iostream>

class Respond {
	public:
		Respond();										//Default constructor
		~Respond();										//Destructor
		Respond(const Respond &other);				//Copy constructor
		Respond &operator=(const Respond &other);	//Copy operator
	private:

};
#endif
