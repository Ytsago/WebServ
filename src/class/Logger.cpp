#include "Logger.hpp"

LogLine::LogLine(std::ostream& os) : _os(os) {}

LogLine::~LogLine() {
	if (Logger::getColor()) _os << RESET << std::endl;
	else _os << std::endl;
}

//-------------Logger-------------//

Logger::Logger(): logs(std::cout), errLogs(std::cerr), color(true) {}
Logger::Logger(std::ostream& logStream, std::ostream& errStream): logs(logStream), errLogs(errStream) {}
bool	Logger::getColor() {return Logger::getInstance().color;}

Logger& Logger::getInstance() {
	static Logger _instance(std::cout, std::cerr);
	return (_instance);
}

std::ostream&	Logger::out() {
	return Logger::getInstance().logs;
}

std::ostream&	Logger::err() {
	return Logger::getInstance().errLogs;
}

std::ostream&	Logger::log(logLevel level) {
	Logger& instance = getInstance();
	std::ostream&	os = (level == ERROR) ? instance.err() : instance.out();

	if (instance.color) {
		switch (level) {
			case INFO: os << FG_AQUA; break;
			case SETUP: os << FG_SALMON;  break;
			case SUCCESS: os << FG_LIME; break;
			case WARNING: os << FG_AMBER; break;
			case ERROR: os << FG_TOMATO; break;
			case DEBUG: os << FG_CHARCOAL; break;
		}
	}

	switch (level) {
		case INFO: os << "[INFO]: "; break;
		case SETUP: os << "[SETUP]: "; break;
		case SUCCESS: os << "[DONE]: "; break;
		case WARNING: os << "[WARNING]: "; break;
		case ERROR: os << "[ERROR]: "; break;
		case DEBUG: os << "[DEBUG]: "; break;
	}
	return os;
}

LogLine	Logger::record(logLevel level) {
	return LogLine(log(level));
}

Logger::~Logger() {

}
