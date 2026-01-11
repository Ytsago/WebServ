#ifndef AMESSAGE_HPP
# define AMESSAGE_HPP

#include <iostream>
#include <hash_map>

class AMessage {
	public:
		AMessage();										//Default constructor
		~AMessage();										//Destructor
		AMessage(const AMessage &other);				//Copy constructor
		AMessage &operator=(const AMessage &other);	//Copy operator
	private:
		

};
#endif
