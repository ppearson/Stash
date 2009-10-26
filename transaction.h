#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "fixed.h"
#include "datetime.h"
#include <string>
#include <vector>

#include "split_transaction.h"

enum TransactionType
{
	None,
	Deposit,
	Withdrawal,
	Transfer,
	PointOfSale,
	Charge,
	ATM
};

class Transaction
{
public:
	Transaction() { m_Split = false; m_Reconciled = false; }
	Transaction(std::string Description, std::string Payee, std::string Category, fixed Amount, Date date);
	
	bool isReconciled() const { return m_Reconciled; }
	void setReconciled(bool recon) { m_Reconciled = recon; }
	Date Date1() const { return m_Date; }
	void setDate(Date date) { m_Date = date; }
	std::string Category() const { return m_Category; }
	void setCategory(std::string Category) { m_Category = Category; }
	std::string Description() const { return m_Description; }
	void setDescription(std::string Description) { m_Description = Description; }
	std::string Payee() const { return m_Payee; }
	void setPayee(std::string Payee) { m_Payee = Payee; }
	fixed Amount() const { return m_Amount; }
	void setAmount(fixed Amount) { m_Amount = Amount; }
	TransactionType Type() const { return m_Type; }
	void setType(TransactionType type) { m_Type = type; }
	
	void setSplit(bool split) { m_Split = split; }
	bool Split() { return m_Split; }
	
	void addSplit(std::string Description, std::string Payee, std::string Category, fixed Amount);
	SplitTransaction & getSplit(int item) { return m_aSplits[item]; }
	int getSplitCount() { return m_aSplits.size(); }
	void deleteSplit(int split) { m_aSplits.erase(m_aSplits.begin() + split); }
	fixed getSplitTotal();
	
	void Load(std::fstream &stream);
	void Store(std::fstream &stream);

private:
	
	std::string ToString() const;
	
	bool m_Reconciled;
	Date m_Date;
	std::string m_Category;
	std::string m_Description;
	std::string m_Payee;
	fixed m_Amount;
	TransactionType m_Type;
	
	bool m_Split;
	
	std::vector<SplitTransaction> m_aSplits;
};

#endif