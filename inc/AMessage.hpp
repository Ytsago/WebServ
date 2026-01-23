#ifndef AMESSAGE_HPP
# define AMESSAGE_HPP

#include <iostream>
#include <map>
#include <vector>

typedef unsigned char byte;
typedef std::vector<byte> byteVector;
typedef std::map<std::string, std::string> headerMap;

class AMessage {
	public:
		AMessage();										//Default constructor
		virtual ~AMessage();										//Destructor
		AMessage(const AMessage &other);				//Copy constructor
		AMessage &operator=(const AMessage &other);	//Copy operator
	
		// byteVector	buildMsg() const;

		void	setFlag(byte flag);
		void	clearFlag(byte flag);
		const byte&	getFlag() const;
		bool	checkFlag(byte flag) const ;

		bool	eof() const;
		bool	fail() const;
		void	clear();

		byteVector& getRaw();
		void	setRaw(const byteVector& data);
		void	append(char* buffer, size_t size);

		const headerMap&	getHeader() const;
		const byteVector&	getBody() const;

	protected:
		byteVector	_entryLine;
		byteVector	_body;

		headerMap	_headerField;

		static const byte	FLAG_EOF = 1 << 0;
		static const byte	FLAG_FAIL = 1 << 1;
		static const byte	FLAG_INPUT = 1 << 2;
		static const byte	FLAG_ENTRY = 1 << 3;
		static const byte	FLAG_HEADER = 1 << 4;
		static const byte	FLAG_BODY = 1 << 5;

	private:
		byte	flags;
		byteVector	_raw;
};

#endif
