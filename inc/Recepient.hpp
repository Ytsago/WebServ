#ifndef RECEPIENT_HPP
# define RECEPIENT_HPP

#include <iostream>

class Recepient {
	public:
		Recepient();										//Default constructor
		~Recepient();										//Destructor
		Recepient(const Recepient &other);				//Copy constructor
		Recepient &operator=(const Recepient &other);	//Copy operator
	private:

};
#endif
