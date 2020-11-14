/* 
 * Stash:  A Personal Finance app (core).
 * Copyright (C) 2010 Peter Pearson
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

#ifndef OFX_H
#define OFX_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "fixed.h"
#include "date.h"
#include "document.h"

enum OFX_TYPE
{
	OFX_UNKNOWN,
	OFX_SGML,
	OFX_XML
};

class OFXBankAccount
{
public:
	OFXBankAccount() { }
	OFXBankAccount(std::string bankID, std::string accountID, std::string type);
	virtual ~OFXBankAccount() { }
	
	void setBankID(std::string id) { m_bankID = id; }
	void setAccountID(std::string id) { m_accountID = id; }
	void setType(std::string type);
	void setType(Account::Type type) { m_type = type; }
	
	std::string		getBankID() const { return m_bankID; }
	std::string		getAccountID() const { return m_accountID; }
	Account::Type	getType() const { return m_type; }
	
private:
	std::string		m_bankID;
	std::string		m_accountID;
	Account::Type	m_type;
};

static Account::Type StringToAccountType(const std::string &string)
{
	if (string == "SAVINGS")
		return Account::eTypeSavings;
	
	return Account::eTypeChecking;
}

class OFXStatementTransaction
{
public:
	OFXStatementTransaction() { }
	OFXStatementTransaction(const Transaction::Type &type, const Date &date, const fixed &amount, std::string &name, int FITID);
	virtual ~OFXStatementTransaction() { }
	
	void setType(std::string &type);
	void setDate(std::string &date);
	void setAmount(std::string &amount) { m_amount.SetFromString(amount); }
	void setName(std::string &name) { m_name = name; }
	void setMemo(std::string &memo) { m_memo = memo; }
	void setFITID(std::string &FITID) { m_fitid = FITID; }
	
	Transaction::Type	getType() const { return m_type; }
	const Date&			getDate() const { return m_date; }
	fixed				getAmount() const { return m_amount; }
	std::string			getName() const { return m_name; }
	std::string			getMemo() const { return m_memo; }
	std::string			getFITID() const { return m_fitid; }
	
	void writeToSGMLStream(std::fstream &stream);
	void writeToXMLStream(std::fstream &stream);
	
private:
	Transaction::Type	m_type;
	Date				m_date;
	fixed				m_amount;
	std::string			m_name;
	std::string			m_memo;
	std::string			m_fitid;
};

static Transaction::Type StringToTransactionType(const std::string &string)
{
	if (string == "CASH" || string == "ATM")
		return Transaction::ATM;
	else if (string == "DEBIT")
		return Transaction::Debit;
	else if (string == "REPEATPMT")
		return Transaction::StandingOrder;
	else if (string == "DIRECTDEBIT")
		return Transaction::DirectDebit;
	else if (string == "PAYMENT" || string == "POS")
		return Transaction::PointOfSale;
	else if (string == "XFER")
		return Transaction::Transfer;
	else if (string == "SRVCHG")
		return Transaction::Charge;
	else if (string == "DEP" || string == "DIRECTDEP")
		return Transaction::Deposit;
	else if (string == "CHECK")
		return Transaction::Cheque;
	else if (string == "CREDIT")
		return Transaction::Credit;
	
	return Transaction::None;
}

static std::string TransactionTypeToString(Transaction::Type type)
{
	std::string strType;
	switch (type)
	{
		case Transaction::ATM:
			strType = "ATM";
			break;
		case Transaction::StandingOrder:
			strType = "REPEATPMT";
			break;
		case Transaction::DirectDebit:
			strType = "DIRECTDEBIT";
			break;
		case Transaction::PointOfSale:
			strType = "POS";
			break;
		case Transaction::Transfer:
			strType = "XFER";
			break;
		case Transaction::Charge:
			strType = "SRVCHG";
			break;
		case Transaction::Deposit:
			strType = "DEP";
			break;
		case Transaction::Cheque:
			strType = "CHECK";
			break;
		case Transaction::Credit:
			strType = "CREDIT";
			break;
		case Transaction::Debit:
			strType = "DEBIT";
			break;
		default:
			strType = "OTHER";
			break;
	}
	
	return strType;	
}

typedef std::vector<OFXStatementTransaction>::const_iterator OFXStTrIt;

class OFXStatementResponse
{
public:
	OFXStatementResponse() { }
	OFXStatementResponse(const OFXBankAccount &account);
	virtual ~OFXStatementResponse() { }
	
	void addTransaction(std::auto_ptr<OFXStatementTransaction> transaction) { m_aTransactions.push_back(*transaction); }
	
	void setAccount(std::auto_ptr<OFXBankAccount> account) { m_account = *account; }
	void calculateBalance();
	void setBalance(std::string &balance) { m_balance.SetFromString(balance); }
	void setCurrencyCode(std::string &currency) { m_currency = currency; }
	
	void reverseTransactionOrder() { std::reverse(m_aTransactions.begin(), m_aTransactions.end()); }
	
	const OFXBankAccount &getAccount() const { return m_account; }
	
	int getTransactionCount() { return m_aTransactions.size(); }
	const OFXStatementTransaction &getTransaction(int trans) const { return m_aTransactions[trans]; }
	
	const fixed &getBalance() const { return m_balance; }
	
	void addOFXTransactionsForAccount(const Account& account);
	
	void writeToSGMLStream(std::fstream &stream);
	void writeToXMLStream(std::fstream &stream);
	
	inline OFXStTrIt begin() const { return m_aTransactions.begin(); }
	inline OFXStTrIt end() const { return m_aTransactions.end(); }
	
private:
	OFXBankAccount		m_account;
	std::vector<OFXStatementTransaction> m_aTransactions;
	fixed				m_balance;
	std::string			m_currency;
};

class OFXStatementTransactionResponse
{
public:
	OFXStatementTransactionResponse() { }
	OFXStatementTransactionResponse(int number) { m_number = number; }
	void setStatementResponse(std::auto_ptr<OFXStatementResponse> statementResponse) { m_statementResponse = *statementResponse; }
	
	OFXStatementResponse &getStatementResponse()  { return m_statementResponse; }
	
	void addOFXTransactionsForAccount(Account &account) {m_statementResponse.addOFXTransactionsForAccount(account); }
	
	void writeToSGMLStream(std::fstream &stream);
	void writeToXMLStream(std::fstream &stream);
	
private:
	OFXStatementResponse m_statementResponse;
	int m_number;
	
};

class OFXBankMessageResponse
{
	
	
};

typedef std::vector<OFXStatementTransactionResponse>::iterator OFXStTrResIt;

class OFXData
{
public:
	OFXData() { }
	void addStatementTransactionResponse(std::auto_ptr<OFXStatementTransactionResponse> pResponse) { m_aStatements.push_back(*pResponse.get()); }
	void addStatementTransactionResponse(OFXStatementTransactionResponse &response) { m_aStatements.push_back(response); }

	inline OFXStTrResIt begin() { return m_aStatements.begin(); }
	inline OFXStTrResIt end() { return m_aStatements.end(); }
	
	OFXStatementTransactionResponse &getResponse(int response) { return m_aStatements[response]; }
	
	int getResponseCount() { return m_aStatements.size(); }
	
	bool exportDataToFile(std::string path, bool xml);
	void writeSGMLStartToStream(std::fstream &stream);
	void writeXMLStartToStream(std::fstream &stream);
	
	void writeEndToStream(std::fstream &stream);
	
private:
	std::vector<OFXStatementTransactionResponse> m_aStatements;
	
};

bool detectOFXType(std::string path, OFX_TYPE &eType, std::string &encoding, std::string &charset);

bool importOFXFile(std::string path, OFXData &dataItem);

bool importOFXSGMLFile(std::string path, OFXData &dataItem, std::string &encoding, std::string &charset);

bool importOFXStatementIntoAccount(Account &account, OFXStatementResponse &statement, bool reverseTransactions, bool cleared,
								   bool ignoreDuplicates);


#endif
