#include "WebServ.hpp"
#include <fstream>

//Experience for reading a binary file
void	fileReader() {
	std::filebuf file;

	void* pt = file.open("/home/secros/Documents/retro-mfa/MFA/blue.mfa", std::ios_base::binary | std::ios_base::in);
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
	std::cout << "Writing output" << std::endl;
	file.open("./new.png", std::ios_base::binary | std::ios_base::out);

	file.sputn(reinterpret_cast<char*>(data.data()), data.size());
}

int main(int ac, char* const av[]) {
	(void) ac, (void)av;
	std::ofstream	logs("logs.txt");
	WebServ server(std::cout, std::cerr);

	server.run();
	// fileReader();
}
