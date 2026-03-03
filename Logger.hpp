#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include "color.h"

enum	logLevel {
	INFO,
	SETUP,
	SUCCESS,
	WARNING,
	ERROR,
	DEBUG,
};

class LogLine {
	private:
		std::ostream& _os;
	public:
		LogLine(std::ostream& os);
		~LogLine();
		
		template <typename T>
		LogLine& operator<<(const T& msg) {
			_os << msg;
			return *this;	
		}
};

class Logger {
	private:
		Logger();
		Logger(const Logger& other);
		Logger(std::ostream& logStream, std::ostream& errStream);
		Logger&	operator=(const Logger& other);
		~Logger();

		std::ostream&	logs;
		std::ostream&	errLogs;
		bool			color;
	public:
		static Logger&	getInstance();
		static std::ostream&	out();
		static std::ostream&	err();
		static std::ostream&	log(logLevel level);
		static LogLine	record(logLevel level);
		static bool	getColor();
};

#endif
