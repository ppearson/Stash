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

#ifndef SCHEDULED_TRANSACTION_H
#define SCHEDULED_TRANSACTION_H

#include "fixed.h"
#include "transaction.h"
#include "datetime.h"
#include <string>

enum Frequency
{
	Weekly,
	TwoWeeks,
	FourWeeks,
	Monthly,
	TwoMonths,
	Quarterly,
	Annually
};

enum Constraint
{
	ExactDate,
	ExactOrNextWorkingDay,
	FirstWorkingDayOfMonth,
	LastWorkingDayOfMonth	
};

class ScheduledTransaction
{
public:
	ScheduledTransaction();
	~ScheduledTransaction() {};
	
	int getAccount() { return m_account; }
	bool isEnabled() { return m_enabled; }
	std::string getPayee() { return m_payee; }
	fixed getAmount() { return m_amount; }
	std::string getCategory() { return m_category; }
	std::string getDescription() { return m_description; }
	Frequency getFrequency() { return m_frequency; }
	Date getNextDate() { return m_nextDate; }
	const Date &getNextDate2() { return m_nextDate; }
	TransactionType getType() { return m_type; }
	Constraint getConstraint() { return m_constraint; }
	
	void setAccount(int account) { m_account = account; }
	void setEnabled(bool enabled) { m_enabled = enabled; }
	void setPayee(std::string payee) { m_payee = payee; }
	void setAmount(fixed amount) { m_amount = amount; }
	void setCategory(std::string category) { m_category = category; }
	void setDescription(std::string description) { m_description = description; }
	void setFrequency(Frequency frequency) { m_frequency = frequency; }
	void setNextDate(Date nextDate) { m_nextDate = nextDate; }
	void setType(TransactionType type) { m_type = type; }
	void setConstraint(Constraint constraint) { m_constraint = constraint; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);
	
	void AdvanceNextDate();
	
protected:
	int m_account;
	bool m_enabled;
	std::string m_payee;
	fixed m_amount;
	std::string m_category;
	std::string m_description;	
	
	Frequency m_frequency;
	Date m_nextDate;
	TransactionType m_type;
	Constraint m_constraint;
	
};

#endif
