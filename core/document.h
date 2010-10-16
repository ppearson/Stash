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
	void deleteCategory(std::string Category);
	
	int addScheduledTransaction(ScheduledTransaction &schedTransaction);
	ScheduledTransaction &getScheduledTransaction(unsigned int schedTrans) { return m_aScheduledTransactions[schedTrans]; }
	void deleteScheduledTransaction(unsigned int schedTrans) { m_aScheduledTransactions.erase(m_aScheduledTransactions.begin() + schedTrans); }
	
	void disabledScheduledTransactionsForAccount(unsigned int nAccount);
	
	int addGraph(Graph &graph) { m_aGraphs.push_back(graph); return m_aGraphs.size() - 1; }
	Graph &getGraph(unsigned int graph) { return m_aGraphs[graph]; }
	Graph *getGraphPtr(unsigned int graph) { return &m_aGraphs[graph]; }
	unsigned int getGraphCount() { return m_aGraphs.size(); }
	void deleteGraph(unsigned int graph) { m_aGraphs.erase(m_aGraphs.begin() + graph); }
	
	void clear();
	
	bool Load(std::fstream &stream);
	bool Store(std::fstream &stream);
	
	inline std::vector<Account>::iterator AccountBegin() { return m_aAccounts.begin(); }
	inline std::vector<Account>::iterator AccountEnd() { return m_aAccounts.end(); }
	
	inline std::set<std::string>::iterator PayeeBegin() { return m_aPayees.begin(); }
	inline std::set<std::string>::iterator PayeeEnd() { return m_aPayees.end(); }
	
	inline std::set<std::string>::iterator CategoryBegin() { return m_aCategories.begin(); }
	inline std::set<std::string>::iterator CategoryEnd() { return m_aCategories.end(); }
	
	inline std::vector<ScheduledTransaction>::iterator SchedTransBegin() { return m_aScheduledTransactions.begin(); }
	inline std::vector<ScheduledTransaction>::iterator SchedTransEnd() { return m_aScheduledTransactions.end(); }
	
	inline std::vector<Graph>::iterator GraphBegin() { return m_aGraphs.begin(); }
	inline std::vector<Graph>::iterator GraphEnd() { return m_aGraphs.end(); }
	
	fixed getBalance(bool onlyReconciled);
	
	bool hasUnsavedChanges() { return m_unsavedChanges; }
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
