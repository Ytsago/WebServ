#include "WebServ.hpp"
#include "ConfigParser.hpp"
#include <fcntl.h>
#include <fstream>
#include "Request.hpp"
#include <fcntl.h>
#include <unistd.h>

#define STEP 5

void	partialReader(const std::string& path, Request& request) {
	int	fd = open(path.c_str(), O_RDONLY);
	char	buffer[STEP];
	ssize_t	byte;

	while ((byte = read(fd, buffer, STEP)) > 0) {
		request.append(buffer, byte);
		request.processMsg();
	}
	close(fd);
}

void	createTestFile() {
	std::ofstream	file("./TestGround/Header/mozillaHeader");

	file << "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nHelloWorld";
}

//Experience for reading a binary file
std::vector<unsigned char>	fileReader(std::string path) {
	std::filebuf file;

	void* pt = file.open(path.c_str(), std::ios_base::binary | std::ios_base::in);
	if (pt == NULL) {
		std::cerr << "ERROR" << std::endl;
	}

	size_t baseSize = file.in_avail();
	std::cout << "Copy inside vector, size : " << baseSize << std::endl;
	std::vector<unsigned char>	data(baseSize);
	size_t value = file.sgetn(reinterpret_cast<char *>(data.data()), file.in_avail());
	if (value < baseSize)
		std::cerr << "PARTIAL READING /!\\" << std::endl;
	file.close();
	return data;
}

int main(int ac, char* const av[]) {
	std::ofstream	logs("logs.txt");

	if (ac == 2)
	{
		try
		{
			WebServ server(std::cout, std::cerr);
			server.run(av[1]);
		}
		catch (const std::exception& e) 
		{
			std::cout << e.what() << std::endl;
			return (1);
    	}
	}
}
