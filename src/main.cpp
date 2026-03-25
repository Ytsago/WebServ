#include "StatusCode.hpp"
#include "WebServ.hpp"
#include "Logger.hpp"

int main(int ac, char* const av[]) {
	Logger::upColor();
	if (ac <= 2)
	{
		try {
			WebServ	server;
			init_status_map();
			server.run(av[1]);
		}
		catch (const std::exception &e) {
			Logger::record(WARNING) << "Program terminated";
			Logger::record(ERROR) << e.what();
		}
	}
}
