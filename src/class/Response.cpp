#include "Response.hpp"
#include "Request.hpp"
#include "ServerConfig.hpp"

static const char PAT[4] = {'\r', '\n', '\r', '\n'};

Response::Response() : AMessage() {
}

Response::Response(const Response &other) : AMessage(other) {
}

Response	&Response::operator=(const Response &other) {
	this->::AMessage::operator=(other);
	return (*this);
}

void	Response::build_response(ServerConfig &server, Request &request) 
{
	//Check that host corresponds to server name
	// if not host -> bad request
	if (request.get_method() == "GET")
		this->build_get_response(server, request);
	if (request.get_method() == "POST")
		this->build_post_response(server, request);
	if (request.get_method() == "DELETE")
		this->build_delete_response(server, request);
}

void	Response::build_get_response(ServerConfig &server, Request &request)
{
	/**
	 * Find corresponding location
	 * Build header:
	 * 		entry-line: "HTTP/1.1" + " " + "CODE" + " " + "STATUS" + "\r\n"
	 * 		map:
	 * 			Content-Length: body.size()
	 * 			Content-Type: check file extension and find corresponding MIME type (create mime type static map) 
	 * 			Date: timestamp RFC 7231 format
	 * 			Etag ?
	 * 			Connection: "keep-alive" or "close"
	 * 	Body: data
	**/	
}

void	Response::build_post_response(ServerConfig &server, Request &request)
{

}

void	Response::build_delete_response(ServerConfig &server, Request &request)
{

}

// void	Response::build_response(ServerConfig &server, Request &request) {
// 	byteVector	msg(_entryLine);

// 	msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	for (std::map<std::string, std::string>::const_iterator it = _headerField.begin(); it != _headerField.end(); it++) {
// 		msg.insert(msg.end(), it->first.begin(), it->first.end() -1);
// 		msg.push_back(':');
// 		msg.insert(msg.end(), it->second.begin(), it->second.end() -1);
// 		msg.insert(msg.end(), PAT, PAT + 4);
// 	}
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	if (this->checkFlag(BODY))
// 		msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	setRaw(msg);
// }

Response::~Response() {
}
