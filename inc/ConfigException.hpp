#pragma once

class ConfigException : public std::exception {
private:

	std::string _message;

public:
	ConfigException(const std::string &error, int line) 
	{
		std::stringstream ss;
		ss << "Config Error: " << error;
		if (line > 0) 
			ss << " at line " << line;
		_message = ss.str();
	}

	virtual ~ConfigException() throw() {}

	virtual const char* what() const throw() 
	{
		return _message.c_str();
	}
};