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

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <bitset>

#include "transaction.h"
#include "storage.h"

Transaction::Transaction() : m_Type(None),
	m_Cleared(false), m_Flagged(false), m_Reconciled(false),
	m_HasFITID(false),
	m_Split(false)
{
	
}

Transaction::Transaction(const std::string& Description, const std::string& Payee, const std::string& Category, fixed Amount, const Date& date) :
	m_Category(Category), m_Description(Description), m_Payee(Payee),
	m_Amount(Amount), 
	m_Date(date),
	m_Type(None),
	m_Cleared(false), m_Flagged(false), m_Reconciled(false),
	m_HasFITID(false),
	m_Split(false)
{
	
}

void Transaction::Load(std::fstream &stream, int version)
{
	m_Date.Load(stream, version);
	Storage::LoadString(m_Description, stream);
	Storage::LoadString(m_Payee, stream);
	Storage::LoadString(m_Category, stream);
	m_Amount.Load(stream, version);
	
	m_Type = (Type)Storage::loadValueFromUChar(stream);
	
	unsigned char cBitset = 0;
	Storage::loadUChar(cBitset, stream);
	
	std::bitset<8> localset(static_cast<unsigned long>(cBitset));
		
	m_Cleared = localset[0];
	m_Split = localset[1];
	
	if (version > 3)
	{
		m_Reconciled = localset[2];
		m_Flagged = localset[3];
		m_HasFITID = localset[4];
		
		if (m_HasFITID)
		{
			Storage::LoadString(m_FITID, stream);
		}
	}

	// old document versions always saved the number of splits, even if we knew
	// there weren't any.
	if (version < 6 || m_Split)
	{
		unsigned char numSplits = 0;
		Storage::loadUChar(numSplits, stream);
		
		for (int i = 0; i < numSplits; i++)
		{
			SplitTransaction tempSplit;
			tempSplit.Load(stream, version);
			
			m_aSplits.push_back(tempSplit);
		}
	}
}

void Transaction::Store(std::fstream &stream) const
{
	m_Date.Store(stream);
	Storage::StoreString(m_Description, stream);
	Storage::StoreString(m_Payee, stream);
	Storage::StoreString(m_Category, stream);
	m_Amount.Store(stream);
	
	Storage::storeValueToUChar(m_Type, stream);
	
	std::bitset<8> localset;	
	localset[0] = m_Cleared;
	localset[1] = m_Split;
	localset[2] = m_Reconciled;
	localset[3] = m_Flagged;
	localset[4] = m_HasFITID;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	Storage::storeUChar(cBitset, stream);
	
	if (m_HasFITID)
	{
		Storage::StoreString(m_FITID, stream);
	}

	if (m_Split)
	{
		unsigned char numSplits = static_cast<unsigned char>(m_aSplits.size());
		Storage::storeUChar(numSplits, stream);
		
		std::vector<SplitTransaction>::const_iterator it = m_aSplits.begin();
		std::vector<SplitTransaction>::const_iterator itEnd = m_aSplits.end();
		
		for (; it != itEnd; ++it)
		{
			(*it).Store(stream);
		}
	}
}

void Transaction::addSplit(const std::string& Description, const std::string& Payee,
						   const std::string& Category, fixed Amount)
{
	SplitTransaction split(Description, Payee, Category, Amount);
	
	m_aSplits.push_back(split);
	m_Split = true;
}

fixed Transaction::getSplitTotal() const
{
	fixed total = 0.0;
	
	std::vector<SplitTransaction>::const_iterator it = m_aSplits.begin();
	
	for (; it != m_aSplits.end(); ++it)
	{
		total += (*it).getAmount();
	}
	
	return total;	
}


