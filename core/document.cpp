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

#include "document.h"

#include "storage.h"

static const int kDocumentVersion = 6;

Document* Document::m_pInstance = NULL;

Document::Document() : m_unsavedChanges(false)
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
	Storage::loadUChar(fileID, stream);
	
	if (fileID != 42)
	{		
		return false;
	}
	
	unsigned char fileVersion = 0;
	Storage::loadUChar(fileVersion, stream);
	
	if (fileVersion > kDocumentVersion)
	{
		// we can't read this file as we don't know about it
		futureVersion = true;
		return false;
	}
	
	m_aAccounts.clear();
	
	unsigned int numAccounts = 0;	
	Storage::loadUInt(numAccounts, stream);
	
	for (unsigned int i = 0; i < numAccounts; i++)
	{
		Account tempAccount;
		tempAccount.Load(stream, fileVersion);
		
		m_aAccounts.push_back(tempAccount);
	}
	
	m_aScheduledTransactions.clear();
	
	unsigned int numSchedTrans = 0;	
	Storage::loadUInt(numSchedTrans, stream);
	
	for (unsigned int i = 0; i < numSchedTrans; i++)
	{
		ScheduledTransaction tempSchedTrans;
		tempSchedTrans.Load(stream, fileVersion);
		
		m_aScheduledTransactions.push_back(tempSchedTrans);
	}	
	
	m_aPayees.clear();
	
	unsigned int numPayees = 0;
	Storage::loadUInt(numPayees, stream);
		
	for (unsigned int i = 0; i < numPayees; i++)
	{
		std::string strPayee;
		Storage::LoadString(strPayee, stream);
		
		m_aPayees.insert(strPayee);
	}
	
	m_aCategories.clear();
	
	unsigned int numCategories = 0;
	Storage::loadUInt(numCategories, stream);
	
	for (unsigned int i = 0; i < numCategories; i++)
	{
		std::string strCategory;
		Storage::LoadString(strCategory, stream);
		
		m_aCategories.insert(strCategory);
	}
	
	m_aGraphs.clear();
	
	if (fileVersion > 1)
	{
		unsigned int numGraphs = 0;
		Storage::loadUInt(numGraphs, stream);
		
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
	Storage::storeUChar(fileID, stream);
	
	unsigned char fileVersion = kDocumentVersion;
	Storage::storeUChar(fileVersion, stream);
	
	unsigned int numAccounts = static_cast<unsigned int>(m_aAccounts.size());
	Storage::storeUInt(numAccounts, stream);
	
	for (std::vector<Account>::const_iterator it = m_aAccounts.begin(); it != m_aAccounts.end(); ++it)
	{
		(*it).Store(stream);
	}
	
	unsigned int numSchedTrans = static_cast<unsigned int>(m_aScheduledTransactions.size());	
	Storage::storeUInt(numSchedTrans, stream);
	
	for (std::vector<ScheduledTransaction>::const_iterator it = m_aScheduledTransactions.begin(); it != m_aScheduledTransactions.end(); ++it)
	{
		(*it).Store(stream);
	}
	
	unsigned int numPayees = static_cast<unsigned int>(m_aPayees.size());
	Storage::storeUInt(numPayees, stream);
	
	for (std::set<std::string>::const_iterator it = m_aPayees.begin(); it != m_aPayees.end(); ++it)
	{
		Storage::StoreString((*it), stream);
	}
	
	unsigned int numCategories = static_cast<unsigned int>(m_aCategories.size());
	Storage::storeUInt(numCategories, stream);
	
	for (std::set<std::string>::const_iterator it = m_aCategories.begin(); it != m_aCategories.end(); ++it)
	{
		Storage::StoreString((*it), stream);
	}
	
	unsigned int numGraphs = static_cast<unsigned int>(m_aGraphs.size());	
	Storage::storeUInt(numGraphs, stream);
	
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

unsigned int Document::addScheduledTransaction(const ScheduledTransaction& schedTransaction)
{
	m_aScheduledTransactions.push_back(schedTransaction);
	return m_aScheduledTransactions.size() - 1u;
}

void Document::disabledScheduledTransactionsForAccountByIndex(unsigned int accountIndex)
{
	for (std::vector<ScheduledTransaction>::iterator it = m_aScheduledTransactions.begin(); it != m_aScheduledTransactions.end(); ++it)
	{
		if (it->getAccount() == accountIndex)
		{
			it->setAccount(-1u);
			it->setEnabled(false);
		}
		
		// also try to re-set the accounts if there's a hole because of the removal
		
		if (it->isEnabled())
		{
			unsigned int nLocalAccount = it->getAccount();
			
			if (nLocalAccount > accountIndex)
			{
				nLocalAccount--;
				
				it->setAccount(nLocalAccount);
			}
		}		
	}
}

