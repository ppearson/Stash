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
	StandingOrder,
	DirectDebit,
	PointOfSale,
	Charge,
	ATM
};

class Transaction
{
public:
	Transaction() { m_Split = false; m_Reconciled = false; m_Type = None; }
	Transaction(std::string Description, std::string Payee, std::string Category, fixed Amount, Date date);
	
	bool isReconciled() const { return m_Reconciled; }
	void setReconciled(bool recon) { m_Reconciled = recon; }
	Date getDate() const { return m_Date; }
	const Date &getDate1() { return m_Date; }
	void setDate(Date date) { m_Date = date; }
	std::string getCategory() const { return m_Category; }
	void setCategory(std::string Category) { m_Category = Category; }
	std::string getDescription() const { return m_Description; }
	void setDescription(std::string Description) { m_Description = Description; }
	std::string getPayee() const { return m_Payee; }
	void setPayee(std::string Payee) { m_Payee = Payee; }
	fixed getAmount() const { return m_Amount; }
	void setAmount(fixed Amount) { m_Amount = Amount; }
	TransactionType getType() const { return m_Type; }
	void setType(TransactionType type) { m_Type = type; }
	
	void setSplit(bool split) { m_Split = split; }
	bool isSplit() { return m_Split; }
	
	void addSplit(SplitTransaction &split) { m_aSplits.push_back(split); }
	void addSplit(std::string Description, std::string Payee, std::string Category, fixed Amount);
	SplitTransaction & getSplit(int item) { return m_aSplits[item]; }
	int getSplitCount() { return m_aSplits.size(); }
	void deleteSplit(int split) { m_aSplits.erase(m_aSplits.begin() + split); }
	fixed getSplitTotal();
	
	void Load(std::fstream &stream, int version);
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