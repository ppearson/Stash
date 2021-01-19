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

#include "document.h"

#include "string.h"

static const int kDocumentVersion = 6;

Document* Document::m_pInstance = NULL;

Document::Document()
{	
	m_pInstance = this;
}

Document* Document::getInstance()
{
	return m_pInstance;
}

bool Document::Load(std::fstream &stream, bool& futureVersion)
{
	unsigned char fileID = 0;
	stream.read((char *) &fileID, sizeof(unsigned char));
	
	if (fileID != 42)
	{		
		return false;
	}
	
	unsigned char fileVersion = 0;
	stream.read((char *) &fileVersion, sizeof(unsigned char));
	
	if (fileVersion > kDocumentVersion)
	{
		// we can't read this file as we don't know about it
		futureVersion = true;
		return false;
	}
	
	m_aAccounts.clear();
	
	unsigned int numAccounts = 0;	
	stream.read((char *) &numAccounts, sizeof(unsigned int));
	
	for (unsigned int i = 0; i < numAccounts; i++)
	{
		Account tempAccount;
		tempAccount.Load(stream, fileVersion);
		
		m_aAccounts.push_back(tempAccount);
	}
	
	m_aScheduledTransactions.clear();
	
	unsigned int numSchedTrans = 0;	
	stream.read((char *) &numSchedTrans, sizeof(unsigned int));
	
	for (unsigned int i = 0; i < numSchedTrans; i++)
	{
		ScheduledTransaction tempSchedTrans;
		tempSchedTrans.Load(stream, fileVersion);
		
		m_aScheduledTransactions.push_back(tempSchedTrans);
	}	
	
	m_aPayees.clear();
	
	unsigned int numPayees = 0;
	stream.read((char *) &numPayees, sizeof(unsigned int));
		
	for (unsigned int i = 0; i < numPayees; i++)
	{
		std::string strPayee;
		LoadString(strPayee, stream);
		
		m_aPayees.insert(strPayee);
	}
	
	m_aCategories.clear();
	
	unsigned int numCategories = 0;
	stream.read((char *) &numCategories, sizeof(unsigned int));
	
	for (unsigned int i = 0; i < numCategories; i++)
	{
		std::string strCategory;
		LoadString(strCategory, stream);
		
		m_aCategories.insert(strCategory);
	}
	
	m_aGraphs.clear();
	
	if (fileVersion > 1)
	{
		unsigned int numGraphs = 0;
		stream.read((char *) &numGraphs, sizeof(unsigned int));
		
		for (unsigned int i = 0; i < numGraphs; i++)
		{
			Graph tempGraph;
			tempGraph.Load(stream, fileVersion);
			
			m_aGraphs.push_back(tempGraph);
		}
	}
	
	return true;
}

bool Document::Store(std::fstream &stream) const
{
	unsigned char fileID = 42;
	stream.write((char *) &fileID, sizeof(unsigned char));
	
	unsigned char fileVersion = kDocumentVersion;
	stream.write((char *) &fileVersion, sizeof(unsigned char));
	
	unsigned int numAccounts = static_cast<unsigned int>(m_aAccounts.size());	
	stream.write((char *) &numAccounts, sizeof(unsigned int));
	
	for (std::vector<Account>::const_iterator it = m_aAccounts.begin(); it != m_aAccounts.end(); ++it)
	{
		(*it).Store(stream);
	}
	
	unsigned int numSchedTrans = static_cast<unsigned int>(m_aScheduledTransactions.size());	
	stream.write((char *) &numSchedTrans, sizeof(unsigned int));
	
	for (std::vector<ScheduledTransaction>::const_iterator it = m_aScheduledTransactions.begin(); it != m_aScheduledTransactions.end(); ++it)
	{
		(*it).Store(stream);
	}
	
	unsigned int numPayees = static_cast<unsigned int>(m_aPayees.size());
	stream.write((char *) &numPayees, sizeof(unsigned int));
	
	for (std::set<std::string>::const_iterator it = m_aPayees.begin(); it != m_aPayees.end(); ++it)
	{
		StoreString((*it), stream);
	}
	
	unsigned int numCategories = static_cast<unsigned int>(m_aCategories.size());
	stream.write((char *) &numCategories, sizeof(unsigned int));
	
	for (std::set<std::string>::const_iterator it = m_aCategories.begin(); it != m_aCategories.end(); ++it)
	{
		StoreString((*it), stream);
	}
	
	unsigned int numGraphs = static_cast<unsigned int>(m_aGraphs.size());	
	stream.write((char *) &numGraphs, sizeof(unsigned int));
	
	for (std::vector<Graph>::const_iterator it = m_aGraphs.begin(); it != m_aGraphs.end(); ++it)
	{
		(*it).Store(stream);
	}
	
	return true;
}

void Document::clear()
{
	m_aAccounts.clear();
	m_aScheduledTransactions.clear();
	m_aPayees.clear();
	m_aCategories.clear();
	m_aGraphs.clear();
}

fixed Document::getBalance(bool onlyReconciled) const
{
	fixed balance = 0.0;
	
	std::vector<Account>::const_iterator it = m_aAccounts.begin();
	
	for (; it != m_aAccounts.end(); ++it)
	{			
		balance += (*it).getBalance(onlyReconciled);
	}
	
	return balance;
}

bool Document::doesPayeeExist(const std::string& payee) const
{
	std::set<std::string>::iterator it = m_aPayees.find(payee);
	
	if (it == m_aPayees.end())
		return false;
	
	return true;
}

void Document::deletePayee(const std::string& payee)
{
	std::set<std::string>::iterator it = m_aPayees.find(payee);
	
	if (it != m_aPayees.end())
		m_aPayees.erase(it);
}

bool Document::doesCategoryExist(const std::string& Category) const
{
	std::set<std::string>::iterator it = m_aCategories.find(Category);
	
	if (it == m_aCategories.end())
		return false;
	
	return true;
}

void Document::deleteCategory(const std::string& Category)
{
	std::set<std::string>::iterator it = m_aCategories.find(Category);
	
	if (it != m_aCategories.end())
		m_aCategories.erase(it);
}

int Document::addScheduledTransaction(const ScheduledTransaction& schedTransaction)
{
	m_aScheduledTransactions.push_back(schedTransaction);
	return m_aScheduledTransactions.size() - 1;
}

void Document::disabledScheduledTransactionsForAccount(unsigned int nAccount)
{
	for (std::vector<ScheduledTransaction>::iterator it = m_aScheduledTransactions.begin(); it != m_aScheduledTransactions.end(); ++it)
	{
		if (it->getAccount() == nAccount)
		{
			it->setAccount(-1);
			it->setEnabled(false);
		}
		
		// also try to re-set the accounts if there's a hole because of the removal
		
		if (it->isEnabled())
		{
			unsigned int nLocalAccount = it->getAccount();
			
			if (nLocalAccount > nAccount)
			{
				nLocalAccount--;
				
				it->setAccount(nLocalAccount);
			}
		}		
	}
}

