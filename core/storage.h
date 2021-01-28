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

#ifndef STORAGE_H
#define STORAGE_H

#include <cstring>
#include <string>
#include <fstream>
#include <stdint.h>

class Storage
{
public:
	Storage()
	{
		
	}
	
	// this load version returns by value, so can be used to read in enums
	// or cast to other values conveniently...
	static unsigned char loadValueFromUChar(std::fstream& stream);
	// similarly, this takes the value by value, so can do implicit casting between types
	// more easily...
	static void storeValueToUChar(const unsigned int value, std::fstream& stream);
	
	static void loadUChar(unsigned char& value, std::fstream& stream);
	static void storeUChar(const unsigned char& value, std::fstream& stream);
	
	static void loadInt(int& value, std::fstream& stream);
	static void storeInt(const int& value, std::fstream& stream);
	
	static void loadUInt(unsigned int& value, std::fstream& stream);
	static void storeUInt(const unsigned int& value, std::fstream& stream);
	
	static void LoadString(std::string& string, std::fstream& stream);
	static void StoreString(const std::string& string, std::fstream& stream);
	
	
};

#endif
