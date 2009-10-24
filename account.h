#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "transaction.h"

class Account
{
public:
	Account();
	~Account() { };
	
	void setName(std::string name) { m_name = name; }
	void setInstitution(std::string institution) { m_institution = institution; }
	void setNumber(std::string number) { m_number = number; }
	
	std::string getName() { return m_name; }
	std::string getInstitution() { return m_institution; }
	std::string getNumber() { return m_number; }
	
	void addTransaction(Transaction &trans) { m_aTransactions.push_back(trans); }
	Transaction &getTransaction(int trans) { return m_aTransactions[trans]; }
	int getTransactionCount() { return m_aTransactions.size(); }
	void deleteTransaction(int trans) { m_aTransactions.erase(m_aTransactions.begin() + trans); }
	
	void Load(std::fstream &stream);
	void Store(std::fstream &stream);
	
	std::vector<Transaction>::iterator begin() { return m_aTransactions.begin(); }
	std::vector<Transaction>::iterator end() { return m_aTransactions.end(); }

protected:
	std::string m_name;
	std::string m_institution;
	std::string m_number;
	
	std::vector<Transaction> m_aTransactions;
};

#endif
