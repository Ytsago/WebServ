#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include "AMessage.hpp"

class Response : public AMessage{
	public:
		Response();										//Default constructor
		~Response();										//Destructor
		Response(const Response &other);				//Copy constructor
		Response &operator=(const Response &other);	//Copy operator
	
		void	build_response(ServerConfig &server, Request &request);
		void	build_get_response(ServerConfig &server, Request &request);
		void	build_post_response(ServerConfig &server, Request &request);
		void	build_delete_response(ServerConfig &server, Request &request);
	private:

		
};
#endif
