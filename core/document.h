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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <set>
#include "account.h"
#include "scheduled_transaction.h"
#include "graph.h"

class Document
{
public:
	Document();
	
	static Document* getInstance();
	
	int addAccount(const Account& acc)
	{
		m_aAccounts.push_back(acc);
		return m_aAccounts.size() - 1;
	}

	Account& getAccountByIndex(unsigned int accIndex)
	{
		return m_aAccounts[accIndex];
	}

	const Account& getAccountByIndex(unsigned int accIndex) const
	{
		return m_aAccounts[accIndex];
	}

	unsigned int getAccountCount() const
	{
		return m_aAccounts.size();
	}

	void deleteAccountByIndex(unsigned int accIndex)
	{
		m_aAccounts.erase(m_aAccounts.begin() + accIndex);
	}
	
	bool doesPayeeExist(const std::string& payee) const;
	void addPayee(const std::string& payee)
	{
		m_aPayees.insert(payee);
	}
	void deletePayee(const std::string& payee);
	
	bool doesCategoryExist(const std::string& Category) const;
	void addCategory(const std::string& Category)
	{
		m_aCategories.insert(Category);
	}
	void deleteCategory(const std::string& Category);
	
	unsigned int addScheduledTransaction(const ScheduledTransaction& schedTransaction);
	ScheduledTransaction& getScheduledTransactionByIndex(unsigned int schedTransIndex)
	{
		return m_aScheduledTransactions[schedTransIndex];
	}
	const ScheduledTransaction& getScheduledTransactionByIndex(unsigned int schedTransIndex) const
	{
		return m_aScheduledTransactions[schedTransIndex];
	}

	void deleteScheduledTransactionByIndex(unsigned int schedTransIndex)
	{
		m_aScheduledTransactions.erase(m_aScheduledTransactions.begin() + schedTransIndex);
	}
	
	void disabledScheduledTransactionsForAccountByIndex(unsigned int accountIndex);
	
	int addGraph(const Graph& graph)
	{
		m_aGraphs.push_back(graph);
		return m_aGraphs.size() - 1;
	}

	Graph& getGraphByIndex(unsigned int graphIndex)
	{
		return m_aGraphs[graphIndex];
	}
	const Graph& getGraphByIndex(unsigned int graphIndex) const
	{
		return m_aGraphs[graphIndex];
	}

	unsigned int getGraphCount() const
	{
		return m_aGraphs.size();
	}
	void deleteGraphByIndex(unsigned int graphIndex)
	{
		m_aGraphs.erase(m_aGraphs.begin() + graphIndex);
	}
	
	void clear();
	
	bool Load(std::fstream& stream, bool& futureVersion);
	bool Store(std::fstream& stream) const;

	const std::vector<Account>& getAccounts() const
	{
		return m_aAccounts;
	}
	
	const std::vector<Graph>& getGraphs() const
	{
		return m_aGraphs;
	}
	
	inline std::vector<Account>::const_iterator AccountBegin() const { return m_aAccounts.begin(); }
	inline std::vector<Account>::const_iterator AccountEnd() const { return m_aAccounts.end(); }
	
	inline std::set<std::string>::const_iterator PayeeBegin() const { return m_aPayees.begin(); }
	inline std::set<std::string>::const_iterator PayeeEnd() const { return m_aPayees.end(); }
	
	inline std::set<std::string>::const_iterator CategoryBegin() const { return m_aCategories.begin(); }
	inline std::set<std::string>::const_iterator CategoryEnd() const { return m_aCategories.end(); }
	
	inline std::vector<ScheduledTransaction>::const_iterator SchedTransBegin() const { return m_aScheduledTransactions.begin(); }
	inline std::vector<ScheduledTransaction>::const_iterator SchedTransEnd() const { return m_aScheduledTransactions.end(); }
	
	inline std::vector<Graph>::const_iterator GraphBegin() const { return m_aGraphs.begin(); }
	inline std::vector<Graph>::const_iterator GraphEnd() const { return m_aGraphs.end(); }
	
	fixed getBalance(bool onlyReconciled) const;
	
	bool hasUnsavedChanges() const { return m_unsavedChanges; }
	void setUnsavedChanges(bool value) { m_unsavedChanges = value; }
	
protected:
	std::vector<Account> m_aAccounts;	
	std::set<std::string> m_aPayees;
	std::set<std::string> m_aCategories;
	std::vector<ScheduledTransaction> m_aScheduledTransactions;
	std::vector<Graph> m_aGraphs;
	
	bool m_unsavedChanges;
	
	static Document* m_pInstance;
	
};

#endif
