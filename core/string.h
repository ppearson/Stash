/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009 Peter Pearson
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

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

inline void StoreString(const std::string& string, std::fstream& stream)
{
	int size = string.size();
	
	if (size <= 255)
	{
		const unsigned char csize = (unsigned char)size;
		
		stream.write((char *) &csize, sizeof(unsigned char));
		stream.write(string.c_str(), csize);
	}
	else // cap the string at 255
	{
		std::string strLimitedString = string.substr(0, 255);
		
		const unsigned char csize = (unsigned char)255;
		
		stream.write((char *) &csize, sizeof(unsigned char));
		stream.write(strLimitedString.c_str(), csize);
	}
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

