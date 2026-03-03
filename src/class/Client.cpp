#include "Client.hpp"
#include "Logger.hpp"
#include "ANetContainer.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client() : ANetContainer(), index(0), _response(NULL) {
}

Client::Client(std::ostream& logs, std::ostream& errLogs) : ANetContainer(logs, errLogs), index(0), _response(NULL) {
}

Client::Client(const Client &other) : ANetContainer(other), index(other.index), _response(other._response) {}

int	Client::get_type() const {
	return (CLIENT);
}
HttpParser&	Client::getParser() {return (this->parser);}


Client	&Client::operator=(const Client &other) {
	if (this != &other) {
		this->::ANetContainer::operator=(other);
		this->index = other.index;
		// this->response = other.response;
	}
	return (*this);
}

const size_t&	Client::getIndex() const {return index;}
Response	*Client::getResponse() {return _response;}
void	Client::setIndex(size_t newIndex) {index = newIndex;}
void	Client::setResponse(Response &response) {
	this->_response = &response;
}


Client::~Client() {
}

void	Client::handleRead() 
{
	size_t	bytes;
	char	buffer[BUFFSIZE];

	bytes = recv(this->_fd, buffer, BUFFSIZE, MSG_DONTWAIT);
	if (bytes <= 0) 
	{
		Logger::err() << "Failed to read msg from client " << _fd << ". Closing connection..." << std::endl;
		this->_state = END;
        return;
	}
	if (this->_state == READING_REQUEST)
	{
		try 
		{
			this->_parser.consume(buffer, bytes);
            if (this->_parser.areHeadersComplete()) 
            {
                if (this->_parser.isMultipart()) 
                {
                    this->_state = WRITING_BODY;
                    this->_fileHandler = new FileHandler(this->_parser.getRequest(), ...);
                    std::vector<char> alreadyReadBody = this->_parser.getBody();
                    if (!alreadyReadBody.empty()) {
                        this->_fileHandler->multiparse(alreadyReadBody);
                        this->_parser.clearBody(); // On vide la RAM du parser
                    }
                } else {
                    this->_state = PROCESSING;
                }
            }
		}
		catch (HttpParser::HttpRequestParsingException &e) 
		{
			Logger::err() << e.what();
			this->_state = END;
		}
	}
	if (this->_state == WRITING_BODY) {
        this->_fileHandler->multiparse(vector_de_char_recu);
        if (_fileHandler->getState() == END)
		{
            this->_state = PROCESSING;
			
		}
    }
}

void Client::handleRead() 
{
    if (this->_state == READING_REQUEST) 
	{

        // 1. On donne les octets au Parser
        // 2. Si le Parser dit "Headers complets" :
        //    - On passe en PROCESSING
        //    - On instancie le RequestHandler
    }
    
    if (this->_state == WRITING_BODY) {
        // C'est ici que ton FileHandler survit !
        // On ne recrée pas l'objet, on appelle juste multiparse
        this->_fileHandler->multiparse(vector_de_char_recu);
        
        if (this->_fileHandler->getState() == END) {
            // Enfin, on peut générer la réponse
            this->_state = PROCESSING; 
        }
    }
}

void Client::handleWrite() {
    if (_state != SENDING_RESPONSE || !_response)
        return;

    std::string& resStr = _response->get_full_response();
    size_t toSend = resStr.size() - _bytesSent;

    ssize_t sent = send(this->_fd, resStr.c_str() + _bytesSent, toSend, 0);
    
    if (sent > 0) {
        _bytesSent += sent;
        if (_bytesSent >= resStr.size()) {
            this->_state = END; // On a tout envoyé !
        }
    } else if (sent == -1) {
        this->_state = END; // Erreur réseau
    }
}
