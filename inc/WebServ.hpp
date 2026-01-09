#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>

class WebServ {
	public:
		WebServ();										//Default constructor
		WebServ(std::ostream& logStream, std::ostream& errorStream);
		~WebServ();										//Destructor
		WebServ(const WebServ &other);				//Copy constructor
		WebServ &operator=(const WebServ &other);	//Copy operator

		bool	run() const;
	private:
		std::ostream& logs;
		std::ostream& errorLogs;
};
#endif
