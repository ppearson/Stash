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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "transaction.h"

enum AccountType
{
	Cash,
	Checking,
	Savings,
	CreditCard,
	Investment,
	Asset,
	Liability,
	Other
};

class Account
{
public:
	Account();
	~Account() { };
	
	void setName(std::string name) { m_name = name; }
	void setInstitution(std::string institution) { m_institution = institution; }
	void setNumber(std::string number) { m_number = number; }
	void setNote(std::string note) { m_note = note; }
	void setType(AccountType type) { m_type = type; }
	
	std::string getName() { return m_name; }
	std::string getInstitution() { return m_institution; }
	std::string getNumber() { return m_number; }
	std::string getNote() { return m_note; }
	AccountType getType() { return m_type; }
	
	int addTransaction(Transaction &trans) { m_aTransactions.push_back(trans); return m_aTransactions.size() - 1; }
	Transaction &getTransaction(int trans) { return m_aTransactions[trans]; }
	int getTransactionCount() { return m_aTransactions.size(); }
	void deleteTransaction(int trans) { m_aTransactions.erase(m_aTransactions.begin() + trans); }
	
	void swapTransactions(int from, int to);
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);
	
	std::vector<Transaction>::iterator begin() { return m_aTransactions.begin(); }
	std::vector<Transaction>::iterator end() { return m_aTransactions.end(); }
	
	fixed getBalance(bool onlyReconciled, int endIndex = -1);

protected:
	std::string m_name;
	std::string m_institution;
	std::string m_number;
	std::string m_note;
	AccountType m_type;
	
	std::vector<Transaction> m_aTransactions;
};

#endif
