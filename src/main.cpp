#include "WebServ.hpp"
#include <fstream>

int main(int ac, char* const av[]) {
	(void) ac, (void)av;
	std::ofstream	logs("logs.txt");
	WebServ server(logs, std::cerr);

	server.run();
}
