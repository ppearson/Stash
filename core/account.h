/*
 * Stash:  A Personal Finance app for OS X.
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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <set>
#include <vector>

#include "transaction.h"

class Account
{
public:
	Account();
	
	enum Type
	{
		eTypeCash,
		eTypeChecking,
		eTypeSavings,
		eTypeCreditCard,
		eTypeInvestment,
		eTypeAsset,
		eTypeLiability,
		eTypeOther
	};
	
	void setName(const std::string& name)
	{
		m_name = name;
	}

	void setInstitution(const std::string& institution)
	{
		m_institution = institution;
	}

	void setNumber(const std::string& number)
	{
		m_number = number;
	}

	void setNote(const std::string& note)
	{
		m_note = note;
	}

	void setType(Type type)
	{
		m_type = type;
	}
	
	const std::string& getName() const { return m_name; }
	const std::string& getInstitution() const { return m_institution; }
	const std::string& getNumber() const { return m_number; }
	const std::string& getNote() const { return m_note; }
	Type getType() const { return m_type; }
	
	int addTransaction(const Transaction& trans, bool bFITID = false);
	Transaction& getTransaction(unsigned int trans) { return m_aTransactions[trans]; }
	const Transaction& getTransaction(unsigned int trans) const { return m_aTransactions[trans]; }
	unsigned int getTransactionCount() const { return m_aTransactions.size(); }
	void deleteTransaction(unsigned int trans) { m_aTransactions.erase(m_aTransactions.begin() + trans); }
	
	bool doesFITIDExist(const std::string& FITID) const;
	void addFITID(const std::string& FITID) { m_aFITIDs.insert(FITID); }
	void deleteFITID(const std::string& FITID);
	
	void swapTransactions(unsigned int from, unsigned int to);
	
	void Load(std::fstream& stream, int version);
	void Store(std::fstream& stream) const;
	
	inline std::vector<Transaction>::iterator begin() { return m_aTransactions.begin(); }
	inline std::vector<Transaction>::iterator end() { return m_aTransactions.end(); }
	inline std::vector<Transaction>::const_iterator begin() const { return m_aTransactions.begin(); }
	inline std::vector<Transaction>::const_iterator end() const { return m_aTransactions.end(); }
	
	fixed getBalance(bool onlyCleared, int endIndex = -1) const;

protected:
	std::string m_name;
	std::string m_institution;
	std::string m_number;
	std::string m_note;
	Type 		m_type;
	
	std::vector<Transaction> m_aTransactions;
	std::set<std::string> m_aFITIDs;
};

#endif
