#ifndef RESPOND_HPP
# define RESPOND_HPP

#include <iostream>
#include "AMessage.hpp"

class Respond : public AMessage{
	public:
		Respond();										//Default constructor
		~Respond();										//Destructor
		Respond(const Respond &other);				//Copy constructor
		Respond &operator=(const Respond &other);	//Copy operator
	
		void	buildMsg();
	private:
};
#endif
