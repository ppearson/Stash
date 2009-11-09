/*
 *  document.h
 *  Stash
 *
 *  Created by Peter Pearson on 28/10/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>
#include "account.h"

class Document
{
public:
	Document();
	~Document() { };
	
	int addAccount(Account &acc) { m_aAccounts.push_back(acc); return m_aAccounts.size() - 1; }
	Account &getAccount(int acc) { return m_aAccounts[acc]; }
	Account *getAccountPtr(int acc) { return &m_aAccounts[acc]; }
	int getAccountCount() { return m_aAccounts.size(); }
	void deleteAccount(int acc) { m_aAccounts.erase(m_aAccounts.begin() + acc); }
	
	bool doesPayeeExist(std::string Payee);
	void addPayee(std::string Payee) { m_aPayees.insert(Payee); }
	void deletePayee(std::string Payee);
	
	bool doesCategoryExist(std::string Category);
	void addCategory(std::string Category) { m_aCategories.insert(Category); }
	
	bool Load(std::fstream &stream);
	bool Store(std::fstream &stream);
	
	std::vector<Account>::iterator AccountBegin() { return m_aAccounts.begin(); }
	std::vector<Account>::iterator AccountEnd() { return m_aAccounts.end(); }
	
	std::set<std::string>::iterator PayeeBegin() { return m_aPayees.begin(); }
	std::set<std::string>::iterator PayeeEnd() { return m_aPayees.end(); }
	
	std::set<std::string>::iterator CategoryBegin() { return m_aCategories.begin(); }
	std::set<std::string>::iterator CategoryEnd() { return m_aCategories.end(); }
	
	fixed getBalance(bool onlyReconciled);
	
protected:
	std::vector<Account> m_aAccounts;	
	std::set<std::string> m_aPayees;
	std::set<std::string> m_aCategories;
	
};

#endif
