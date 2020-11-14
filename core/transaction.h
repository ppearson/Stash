/*
 * Stash:  A Personal Finance app (core).
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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "fixed.h"
#include "date.h"
#include <string>
#include <vector>

#include "split_transaction.h"

class Transaction
{
public:
	Transaction();
	Transaction(const std::string& Description, const std::string& Payee, const std::string& Category, fixed Amount, Date date);

	enum Type
	{
		None,
		Deposit,
		Withdrawal,
		Transfer,
		StandingOrder,
		DirectDebit,
		PointOfSale,
		Charge,
		ATM,
		Cheque,
		Credit,
		Debit
	};
	
	bool isCleared() const { return m_Cleared; }
	void setCleared(bool cleared) { m_Cleared = cleared; }
	bool isFlagged() const { return m_Flagged; }
	void setFlagged(bool flagged) { m_Flagged = flagged; }
	bool isReconciled() const { return m_Reconciled; }
	void setReconciled(bool recon) { m_Reconciled = recon; }
	
	const Date& getDate() const { return m_Date; }
	void setDate(const Date& date) { m_Date = date; }
	std::string getCategory() const { return m_Category; }
	void setCategory(std::string Category) { m_Category = Category; }
	std::string getDescription() const { return m_Description; }
	void setDescription(std::string Description) { m_Description = Description; }
	std::string getPayee() const { return m_Payee; }
	void setPayee(std::string Payee) { m_Payee = Payee; }
	const fixed& getAmount() const { return m_Amount; }
	void setAmount(fixed Amount) { m_Amount = Amount; }
	Type getType() const { return m_Type; }
	void setType(Type type) { m_Type = type; }
	std::string getFITID() const { return m_FITID; }
	void setFITID(std::string FITID) { m_FITID = FITID; }
	
	bool hasFITID() const { return m_HasFITID; }
	void setHasFITID(bool has) { m_HasFITID = has; }
	
	void setSplit(bool split) { m_Split = split; }
	bool isSplit() const { return m_Split; }
	
	void addSplit(SplitTransaction &split) { m_aSplits.push_back(split); }
	void addSplit(std::string Description, std::string Payee, std::string Category, fixed Amount);
	SplitTransaction & getSplit(unsigned int item) { return m_aSplits[item]; }
	const SplitTransaction & getSplit(unsigned int item) const { return m_aSplits[item]; }
	unsigned int getSplitCount() const { return m_aSplits.size(); }
	void deleteSplit(int split) { m_aSplits.erase(m_aSplits.begin() + split); }
	fixed getSplitTotal() const;
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream) const;

private:
	// TODO: memory packing / ordering
	bool			m_Cleared;
	bool			m_Flagged;
	bool			m_Reconciled;
		
	Date			m_Date;
	std::string		m_Category;
	std::string		m_Description;
	std::string		m_Payee;
	fixed			m_Amount;
	Type			m_Type;
	
	bool			m_HasFITID;
	std::string		m_FITID;
	
	bool			m_Split;
	
	std::vector<SplitTransaction> m_aSplits;
};

#endif
