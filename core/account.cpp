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

#include "account.h"
#include "string.h"

Account::Account()
{
	m_type = Cash;
}

void Account::Load(std::fstream &stream, int version)
{
	LoadString(m_name, stream);
	LoadString(m_institution, stream);
	LoadString(m_number, stream);
	LoadString(m_note, stream);
	
	stream.read((char *) &m_type, sizeof(unsigned char));
	
	unsigned int numTransactions = 0;
	
	stream.read((char *) &numTransactions, sizeof(unsigned int));
	
	for (unsigned int i = 0; i < numTransactions; i++)
	{
		Transaction tempTransaction;
		tempTransaction.Load(stream, version);
		
		m_aTransactions.push_back(tempTransaction);
	}
}

void Account::Store(std::fstream &stream)
{
	StoreString(m_name, stream);
	StoreString(m_institution, stream);
	StoreString(m_number, stream);
	StoreString(m_note, stream);
	
	stream.write((char *) &m_type, sizeof(unsigned char));
	
	unsigned int numTransactions = static_cast<unsigned int>(m_aTransactions.size());
	
	stream.write((char *) &numTransactions, sizeof(unsigned int));
	
	for (std::vector<Transaction>::iterator it = m_aTransactions.begin(); it != m_aTransactions.end(); ++it)
	{
		(*it).Store(stream);
	}
}

void Account::swapTransactions(int from, int to)
{
	iter_swap(m_aTransactions.begin() + from, m_aTransactions.begin() + to);
}

fixed Account::getBalance(bool onlyReconciled)
{
	fixed balance = 0.0;
	
	std::vector<Transaction>::iterator it = m_aTransactions.begin();
	
	if (onlyReconciled)
	{
		for (; it != m_aTransactions.end(); ++it)
		{
			if ((*it).isReconciled())
			{
				balance += (*it).Amount();
			}
		}
	}
	else
	{
		for (; it != m_aTransactions.end(); ++it)
		{			
			balance += (*it).Amount();
		}
	}
	
	return balance;
}