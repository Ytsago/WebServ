#include "WebServ.hpp"
#include "ConfigParser.hpp"
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

// int main(int ac, char* const av[]) {
// 	std::ofstream	logs("logs.txt");
	
// 	if (ac == 2)
// 	{
// 		try
// 		{
// 			WebServ server(std::cout, std::cerr);
// 			server.run(av[1]);
// 		}
// 		catch (const std::exception& e) 
// 		{
// 			std::cout << e.what() << std::endl;
// 			return (1);
//     	}
// 	}
// }

int main(int ac, char* const av[]) 
{
	if (ac == 2)
	{
		try
		{
			ConfigParser	parser(av[1]);
			std::cout << parser;
		}
		catch (const std::exception& e) 
		{
			std::cout << e.what() << std::endl;
			return (1);
    	}
	}
}
