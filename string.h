#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

inline void StoreString(const std::string& string, std::fstream& stream)
{
	const unsigned char size = (unsigned char)string.size();
	
	stream.write((char *) &size, sizeof(unsigned char));
	stream.write(string.c_str(), size);

//	if (!out) throw serialization_error("Error serializing object of type std::string");
}

inline void LoadString(std::string& string, std::fstream& stream)
{
	char* buf = 0;
	
	unsigned char size = 0;
	stream.read((char *) &size, sizeof(unsigned char));
	
	buf = new char[size + 1];
	stream.read(buf, size);
	buf[size] = 0;
	
	string.assign(buf);
	
	delete [] buf;
}
