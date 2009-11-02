/*
 *  document.cpp
 *  Stash
 *
 *  Created by Peter Pearson on 28/10/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "document.h"
#include "string.h"

#define FILE_VERSION 0

Document::Document()
{
	
}

bool Document::Load(std::fstream &stream)
{
	unsigned char fileID = 0;
	stream.read((char *) &fileID, sizeof(unsigned char));
	
	if (fileID != 42)
	{		
		return false;
	}
	
	m_aAccounts.clear();
	
	unsigned char fileVersion = 0;
	stream.read((char *) &fileVersion, sizeof(unsigned char));
	
	unsigned int numAccounts = 0;	
	stream.read((char *) &numAccounts, sizeof(unsigned int));
	
	for (unsigned int i = 0; i < numAccounts; i++)
	{
		Account tempAccount;
		tempAccount.Load(stream, fileVersion);
		
		m_aAccounts.push_back(tempAccount);
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
	
	return true;
}

bool Document::Store(std::fstream &stream)
{
	unsigned char fileID = 42;
	stream.write((char *) &fileID, sizeof(unsigned char));
	
	unsigned char fileVersion = FILE_VERSION;
	stream.write((char *) &fileVersion, sizeof(unsigned char));
	
	unsigned int numAccounts = static_cast<unsigned int>(m_aAccounts.size());	
	stream.write((char *) &numAccounts, sizeof(unsigned int));
	
	for (std::vector<Account>::iterator it = m_aAccounts.begin(); it != m_aAccounts.end(); ++it)
	{
		(*it).Store(stream);
	}
	
	unsigned int numPayees = static_cast<unsigned int>(m_aPayees.size());
	stream.write((char *) &numPayees, sizeof(unsigned int));
	
	for (std::set<std::string>::iterator it = m_aPayees.begin(); it != m_aPayees.end(); ++it)
	{
		StoreString((*it), stream);
	}
	
	unsigned int numCategories = static_cast<unsigned int>(m_aCategories.size());
	stream.write((char *) &numCategories, sizeof(unsigned int));
	
	for (std::set<std::string>::iterator it = m_aCategories.begin(); it != m_aCategories.end(); ++it)
	{
		StoreString((*it), stream);
	}
	
	return true;
}

fixed Document::getBalance(bool onlyReconciled)
{
	fixed balance = 0.0;
	
	std::vector<Account>::iterator it = m_aAccounts.begin();
	
	for (; it != m_aAccounts.end(); ++it)
	{			
		balance += (*it).getBalance(onlyReconciled);
	}
	
	return balance;
}

bool Document::doesPayeeExist(std::string Payee)
{
	std::set<std::string>::iterator it = m_aPayees.find(Payee);
	
	if (it == m_aPayees.end())
		return false;
	
	return true;
}

bool Document::doesCategoryExist(std::string Category)
{
	std::set<std::string>::iterator it = m_aCategories.find(Category);
	
	if (it == m_aCategories.end())
		return false;
	
	return true;
}
