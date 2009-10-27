/*
 *  account.cpp
 *  moneytest
 *
 *  Created by Peter Pearson on 24/10/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "account.h"
#include "string.h"

Account::Account()
{

}

void Account::Load(std::fstream &stream, int version)
{
	LoadString(m_name, stream);
	LoadString(m_institution, stream);
	LoadString(m_number, stream);
	LoadString(m_note, stream);
	
	stream.read((char *) &m_type, sizeof(AccountType));
	
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
	
	stream.write((char *) &m_type, sizeof(AccountType));
	
	unsigned int numTransactions = static_cast<unsigned int>(m_aTransactions.size());
	
	stream.write((char *) &numTransactions, sizeof(unsigned int));
	
	for (std::vector<Transaction>::iterator it = m_aTransactions.begin(); it != m_aTransactions.end(); ++it)
	{
		(*it).Store(stream);
	}
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