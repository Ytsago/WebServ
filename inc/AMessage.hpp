#ifndef AMESSAGE_HPP
# define AMESSAGE_HPP

#include <iostream>
#include <map>
#include <vector>

typedef unsigned char byte;
typedef std::vector<char> byteVector;

class AMessage {
	public:
		AMessage();										//Default constructor
		~AMessage();										//Destructor
		AMessage(const AMessage &other);				//Copy constructor
		AMessage &operator=(const AMessage &other);	//Copy operator
	
		// byteVector	buildMsg() const;

		void	setFlag(byte flag);
		const byte&	getFlag() const;
		bool	checkFlag(byte flag) const ;
		bool	eof() const;
		bool	fail() const;
		void	clear();

		byteVector& getRaw();
		void	setRaw(byteVector data);
	protected:
		byteVector	_entryLine;
		byteVector	_body;

		std::map<std::string, std::string> _headerField;

		static const byte	_EOF = 1 << 0;
		static const byte	FAIL = 1 << 1;
		static const byte	INPUT = 1 << 2;
		static const byte	ENTRY = 1 << 3;
		static const byte	HEADER = 1 << 4;
		static const byte	BODY = 1 << 5;
	private:
		byte	flags;
		byteVector	_raw;
};

#endif
