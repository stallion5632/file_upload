#include "http_upload.h"
#include <iostream>
#include <boost/filesystem.hpp>


int main(int argc, char* argv[])
{
	if (argc != 4)
    {
      std::cout << "Usage: http_upload <server> <port> <filepath>\n";
      return 1;
    }

	std::string filepath(argv[3]);
	uint64_t dataSize = boost::filesystem::file_size(filepath);

	//Read file into memory
	FILE * fp;
	fp = fopen(filepath.c_str(), "rb");
	char* array = new char[1024 * 1024 * 40];
	fseek(fp, 0, SEEK_SET);
	fread(array, sizeof(char), dataSize, fp);
	fclose(fp);

	std::string filename;
	int pos = filepath.find_last_of('/');
	if (pos > 0)
		filename = filepath.substr(pos + 1);
	else
		filename = filepath;

	std::string host = argv[1];
	std::string port = argv[2];
	std::string url = "/upload";
	int ret = post(host, port, url, filename, array, dataSize);
	if (0 == ret)
		std::cout << "OK" << std::endl;
	else
		std::cout << "not OK"<< std::endl;
	delete array;
	
	return 0;
}