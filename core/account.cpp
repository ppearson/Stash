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

#include "account.h"

#include <algorithm>

#include "storage.h"

Account::Account()
{
	m_type = eTypeCash;
}

void Account::Load(std::fstream& stream, int version)
{
	Storage::LoadString(m_name, stream);
	Storage::LoadString(m_institution, stream);
	Storage::LoadString(m_number, stream);
	Storage::LoadString(m_note, stream);
	
	m_type = (Type)Storage::loadValueFromUChar(stream);
	
	unsigned int numTransactions = 0;
	Storage::loadUInt(numTransactions, stream);

	m_aTransactions.reserve(numTransactions);
	
	for (unsigned int i = 0; i < numTransactions; i++)
	{
		Transaction tempTransaction;
		tempTransaction.Load(stream, version);
		
		if (tempTransaction.hasFITID())
		{
			std::string strFITID = tempTransaction.getFITID();
			addFITID(strFITID);
		}
		
		m_aTransactions.push_back(tempTransaction);
	}
}

void Account::Store(std::fstream& stream) const
{
	Storage::StoreString(m_name, stream);
	Storage::StoreString(m_institution, stream);
	Storage::StoreString(m_number, stream);
	Storage::StoreString(m_note, stream);
	
	Storage::storeValueToUChar(m_type, stream);
	
	unsigned int numTransactions = static_cast<unsigned int>(m_aTransactions.size());
	Storage::storeUInt(numTransactions, stream);
	
	std::vector<Transaction>::const_iterator it = m_aTransactions.begin();
	std::vector<Transaction>::const_iterator itEnd = m_aTransactions.end();
	
	for (; it != itEnd; ++it)
	{
		(*it).Store(stream);
	}
}

int Account::addTransaction(const Transaction& trans, bool bFITID)
{
	if (bFITID && trans.hasFITID())
	{
		std::string strFITID = trans.getFITID();
		addFITID(strFITID);
	}
	
	m_aTransactions.push_back(trans);
	return m_aTransactions.size() - 1;
}

void Account::swapTransactions(unsigned int from, unsigned int to)
{
	iter_swap(m_aTransactions.begin() + from, m_aTransactions.begin() + to);
}

bool Account::doesFITIDExist(const std::string& FITID) const
{
	std::set<std::string>::iterator it = m_aFITIDs.find(FITID);
	
	if (it == m_aFITIDs.end())
		return false;
	
	return true;
}

void Account::deleteFITID(const std::string& FITID)
{
	std::set<std::string>::iterator it = m_aFITIDs.find(FITID);
	
	if (it != m_aFITIDs.end())
		m_aFITIDs.erase(it);
}

fixed Account::getBalance(bool onlyCleared, int endIndex) const
{
	fixed balance = 0.0;
	
	std::vector<Transaction>::const_iterator it = m_aTransactions.begin();
	std::vector<Transaction>::const_iterator itEnd = m_aTransactions.end();
	
	if (endIndex != -1)
	{
		itEnd = m_aTransactions.begin() + endIndex;
	}
	
	if (onlyCleared)
	{
		for (; it != itEnd; ++it)
		{
			if ((*it).isCleared())
			{
				balance += (*it).getAmount();
			}
		}
	}
	else
	{
		for (; it != itEnd; ++it)
		{			
			balance += (*it).getAmount();
		}
	}
	
	return balance;
}
