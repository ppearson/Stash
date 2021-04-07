/*
 * Stash:  A Personal Finance app (core).
 * Copyright (C) 2009-2021 Peter Pearson
 * You can view the complete license in the Licence.txt file in the root
 * of the source tree.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "storage.h"

#define USE_STACK_BUFFER_FOR_STRINGS 1

// TODO: error checking and return values!

unsigned char Storage::loadValueFromUChar(std::fstream& stream)
{
	unsigned char value = 0;
	stream.read((char *) &value, sizeof(unsigned char));
	return value;
}

void Storage::storeValueToUChar(const unsigned int value, std::fstream& stream)
{
	stream.write((char*)&value, sizeof(unsigned char));
}

void Storage::loadUChar(unsigned char& value, std::fstream& stream)
{
	stream.read((char*)&value, sizeof(unsigned char));
}

void Storage::storeUChar(const unsigned char& value, std::fstream& stream)
{
	stream.write((char*)&value, sizeof(unsigned char));
}

void Storage::loadInt(int& value, std::fstream& stream)
{
	stream.read((char*)&value, sizeof(int));
}

void Storage::storeInt(const int& value, std::fstream& stream)
{
	stream.write((char*)&value, sizeof(int));
}

void Storage::loadUInt(unsigned int& value, std::fstream& stream)
{
	stream.read((char*)&value, sizeof(unsigned int));
}

void Storage::storeUInt(const unsigned int& value, std::fstream& stream)
{
	stream.write((char*)&value, sizeof(unsigned int));
}

// Note: This isn't really used any more (at least for recent file format versions),
//       but it's still here just to be able to read older file format versions correctly
void Storage::loadUInt64(uint64_t& value, std::fstream& stream)
{
	stream.read((char *)&value, sizeof(uint64_t));
}

// Note: This isn't really needed at all, as we can't save older versions, but it's here
//       for completeness
void Storage::storeUInt64(const uint64_t& value, std::fstream& stream)
{
	stream.write((char *)&value, sizeof(uint64_t));
}

void Storage::LoadString(std::string& string, std::fstream& stream)
{
	unsigned char size = 0;
	loadUChar(size, stream);
	
#if USE_STACK_BUFFER_FOR_STRINGS
	char buf[256];
#else
	char* buf = new char[size + 1];
#endif
	stream.read(buf, size);
	buf[size] = 0;
	
	string.assign(buf);
	
#if USE_STACK_BUFFER_FOR_STRINGS
#else
	delete [] buf;
#endif
}

void Storage::StoreString(const std::string& string, std::fstream& stream)
{
	int size = string.size();
	
	if (size <= 255)
	{
		const unsigned char csize = (unsigned char)size;
		
		storeUChar(csize, stream);
		stream.write(string.c_str(), csize);
	}
	else // cap the string at 255
	{
		std::string strLimitedString = string.substr(0, 255);
		
		const unsigned char csize = (unsigned char)255;
		
		storeUChar(csize, stream);
		stream.write(strLimitedString.c_str(), csize);
	}
}
