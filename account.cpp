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

void Account::Load(std::fstream &stream)
{
	LoadString(m_name, stream);
	LoadString(m_institution, stream);
	LoadString(m_number, stream);
	
	unsigned int numTransactions = 0;
	
	stream.read((char *) &numTransactions, sizeof(unsigned int));
	
	for (unsigned int i = 0; i < numTransactions; i++)
	{
		Transaction tempTransaction;
		tempTransaction.Load(stream);
		
		m_aTransactions.push_back(tempTransaction);
	}
}

void Account::Store(std::fstream &stream)
{
	StoreString(m_name, stream);
	StoreString(m_institution, stream);
	StoreString(m_number, stream);
	
	unsigned int numTransactions = static_cast<unsigned int>(m_aTransactions.size());
	
	stream.write((char *) &numTransactions, sizeof(unsigned int));
	
	for (std::vector<Transaction>::iterator it = m_aTransactions.begin(); it != m_aTransactions.end(); ++it)
	{
		(*it).Store(stream);
	}
}