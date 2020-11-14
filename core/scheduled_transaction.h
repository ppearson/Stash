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

#ifndef SCHEDULED_TRANSACTION_H
#define SCHEDULED_TRANSACTION_H

#include "fixed.h"
#include "transaction.h"
#include "date.h"
#include <string>

class ScheduledTransaction
{
public:
	ScheduledTransaction();
	~ScheduledTransaction() {}

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
		LastWorkingDayOfMonth
	};
	
	unsigned int getAccount() const { return m_account; }
	bool isEnabled() const { return m_enabled; }
	const std::string& getPayee() const { return m_payee; }
	fixed getAmount() const { return m_amount; }
	const std::string& getCategory() const { return m_category; }
	const std::string& getDescription() const { return m_description; }
	Frequency getFrequency() const { return m_frequency; }
	const Date& getNextDate() const { return m_nextDate; }
	Transaction::Type getType() const { return m_type; }
	Constraint getConstraint() const { return m_constraint; }
	
	void setAccount(unsigned int account) { m_account = account; }
	void setEnabled(bool enabled) { m_enabled = enabled; }
	void setPayee(std::string payee) { m_payee = payee; }
	void setAmount(fixed amount) { m_amount = amount; }
	void setCategory(std::string category) { m_category = category; }
	void setDescription(std::string description) { m_description = description; }
	void setFrequency(Frequency frequency) { m_frequency = frequency; }
	void setNextDate(Date nextDate) { m_nextDate = nextDate; }
	void setType(Transaction::Type type) { m_type = type; }
	void setConstraint(Constraint constraint) { m_constraint = constraint; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream) const;
	
	void AdvanceNextDate();
	
protected:
	unsigned int		m_account;
	bool				m_enabled;
	std::string			m_payee;
	fixed				m_amount;
	std::string			m_category;
	std::string			m_description;
	
	Frequency			m_frequency;
	Date				m_nextDate;
	Transaction::Type	m_type;
	Constraint			m_constraint;
	
};

#endif
