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

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <bitset>
#include "transaction.h"
#include "string.h"

Transaction::Transaction() : m_Split(false), m_Type(None), m_Cleared(false), m_Flagged(false), m_Reconciled(false), m_HasFITID(false)
{
	
}

Transaction::Transaction(const std::string& Description, const std::string& Payee, const std::string& Category, fixed Amount, Date date) :
	m_Description(Description), m_Payee(Payee), m_Category(Category), m_Amount(Amount), m_Date(date), m_Split(false), m_Type(None),
	m_Cleared(false), m_Reconciled(false), m_Flagged(false), m_HasFITID(false)
{
	
}

void Transaction::Load(std::fstream &stream, int version)
{
	m_Date.Load(stream, version);
	LoadString(m_Description, stream);
	LoadString(m_Payee, stream);
	LoadString(m_Category, stream);
	m_Amount.Load(stream, version);
	
	stream.read((char *) &m_Type, sizeof(unsigned char));
	
	unsigned char cBitset = 0;
	stream.read((char *) &cBitset, sizeof(unsigned char));
	
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
			LoadString(m_FITID, stream);
		}
	}

	// old document versions always saved the number of splits, even if we knew
	// there weren't any.
	if (version < 6 || m_Split)
	{
		unsigned char numSplits = 0;
		stream.read((char *) &numSplits, sizeof(unsigned char));
		
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
	StoreString(m_Description, stream);
	StoreString(m_Payee, stream);
	StoreString(m_Category, stream);
	m_Amount.Store(stream);
	
	stream.write((char *) &m_Type, sizeof(unsigned char));
	
	std::bitset<8> localset;	
	localset[0] = m_Cleared;
	localset[1] = m_Split;
	localset[2] = m_Reconciled;
	localset[3] = m_Flagged;
	localset[4] = m_HasFITID;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	stream.write((char *) &cBitset, sizeof(unsigned char));
	
	if (m_HasFITID)
	{
		StoreString(m_FITID, stream);
	}

	if (m_Split)
	{
		unsigned char numSplits = static_cast<unsigned char>(m_aSplits.size());
		stream.write((char *) &numSplits, sizeof(unsigned char));
		
		std::vector<SplitTransaction>::const_iterator it = m_aSplits.begin();
		std::vector<SplitTransaction>::const_iterator itEnd = m_aSplits.end();
		
		for (; it != itEnd; ++it)
		{
			(*it).Store(stream);
		}
	}
}

void Transaction::addSplit(std::string Description, std::string Payee, std::string Category, fixed Amount)
{
	SplitTransaction split(Description, Payee, Category, Amount);
	
	m_aSplits.push_back(split);
	m_Split = true;
}

fixed Transaction::getSplitTotal() const
{
	fixed total = 0.0;
	
	std::vector<SplitTransaction>::const_iterator it = m_aSplits.begin();
	std::vector<SplitTransaction>::const_iterator itEnd = m_aSplits.end();
	
	for (; it != m_aSplits.end(); ++it)
	{
		total += (*it).getAmount();
	}
	
	return total;	
}


