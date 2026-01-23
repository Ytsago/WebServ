#ifndef SENDER_HPP
# define SENDER_HPP

#include <iostream>

class Sender {
	public:
		static Sender&	getInstance();
		
	private:
		Sender &operator=(const Sender &other);	//Copy operator
		Sender(const Sender &other);				//Copy constructor
		~Sender();										//Destructor
		Sender();										//Default constructor

};
#endif
