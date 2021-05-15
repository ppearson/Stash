/* 
 * Stash:  A Personal Finance app (core).
 * Copyright (C) 2009-2020 Peter Pearson
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

#include <bitset>

#include "graph.h"
#include "storage.h"

Graph::Graph() : m_name(""), m_account(0), m_viewType(Total), m_type(ExpenseCategories), m_ignoreTransfers(false),
			m_dateType(DateCustom), m_itemsType(AllItems)
{

}

void Graph::Load(std::fstream &stream, int version)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 256 accounts...
	m_account = Storage::loadValueFromUChar(stream);
	
	Storage::LoadString(m_name, stream);
	
	m_startDate.Load(stream, version);
	m_endDate.Load(stream, version);
	
	m_type = (Graph::Type)Storage::loadValueFromUChar(stream);
	
	unsigned char cBitset = 0;
	Storage::loadUChar(cBitset, stream);
	
	std::bitset<8> localset(static_cast<uint64_t>(cBitset));
	
	m_ignoreTransfers = localset[0];
	
	if (version > 2)
	{
		m_dateType = (Graph::DateType)Storage::loadValueFromUChar(stream);
	}
	
	if (version > 4)
	{
		m_itemsType = (Graph::ItemsType)Storage::loadValueFromUChar(stream);
		
		m_items.clear();
		
		unsigned int numItems = 0;
		Storage::loadUInt(numItems, stream);
		
		for (unsigned int i = 0; i < numItems; i++)
		{
			std::string strItem;
			Storage::LoadString(strItem, stream);
			
			m_items.insert(strItem);
		}
		
		m_viewType = (Graph::ViewType)Storage::loadValueFromUChar(stream);
	}
}

void Graph::Store(std::fstream &stream) const
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 256 accounts...
	Storage::storeValueToUChar((unsigned char)m_account, stream);
	
	Storage::StoreString(m_name, stream);
	
	m_startDate.Store(stream);
	m_endDate.Store(stream);
	
	Storage::storeValueToUChar(m_type, stream);
	
	std::bitset<8> localset;	
	localset[0] = m_ignoreTransfers;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	Storage::storeUChar(cBitset, stream);
	
	Storage::storeValueToUChar(m_dateType, stream);
	
	Storage::storeValueToUChar(m_itemsType, stream);
	
	unsigned int numItems = static_cast<unsigned int>(m_items.size());
	Storage::storeUInt(numItems, stream);
	
	for (std::set<std::string>::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		Storage::StoreString((*it), stream);
	}
	
	Storage::storeValueToUChar(m_viewType, stream);
}
