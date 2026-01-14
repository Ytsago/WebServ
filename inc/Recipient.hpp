#pragma once 

//Recipient use to receive message from a client,
//it my be usefull to make it a part of the client class
//Beware of fractionnal msg
class Recipient {
	public:
		Recipient() {};
		~Recipient() {};
		static void	getMsg(int fd);
};

//Sender use to send message to a client,
//it my be usefull to make it a part of the client class
//Beware of fractionnal msg
class Sender {
	public:
		static int sendMsg(Client &client);
};