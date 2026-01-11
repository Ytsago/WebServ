#ifndef AMESSAGE_HPP
# define AMESSAGE_HPP

#include <ios>
#include <iostream>
#include <map>
#include <vector>

typedef unsigned char byte;

class AMessage {
	public:
		AMessage();										//Default constructor
		~AMessage();										//Destructor
		AMessage(const AMessage &other);				//Copy constructor
		AMessage &operator=(const AMessage &other);	//Copy operator
	
		std::vector<byte>	buildMsg() const;

		inline void	setFlag(byte flag);
		inline bool	getFlag(byte flag) const ;
		inline void	clear();
		std::ios_base::iostate test;
	protected:
		std::vector<byte>	_entryLine;
		std::vector<byte>	_body;

		std::map<std::string, std::string> _headerField;

		static const byte EOF = 1 << 0;
		static const byte FAIL = 1 << 1;
		static const byte INPUT = 1 << 2;
	private:
		byte	flags;
};

#endif
