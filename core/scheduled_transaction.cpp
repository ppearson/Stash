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

#include <bitset>

#include "scheduled_transaction.h"
#include "storage.h"

ScheduledTransaction::ScheduledTransaction() : m_enabled(true), m_frequency(Weekly), m_account(0), m_type(Transaction::None), m_constraint(ExactDate)
{
	m_nextDate.Now();
}

void ScheduledTransaction::Load(std::fstream &stream, int version)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 256 accounts...
	m_account = Storage::loadValueFromUChar(stream);
	
	if (version == 0)
	{
		m_enabled = true;
	}
	else
	{
		unsigned char cBitset = 0;
		Storage::loadUChar(cBitset, stream);
	
		std::bitset<8> localset(static_cast<unsigned long>(cBitset));
	
		m_enabled = localset[0];
	}
	
	Storage::LoadString(m_payee, stream);
	m_amount.Load(stream, version);
	Storage::LoadString(m_category, stream);
	Storage::LoadString(m_description, stream);
	
	m_frequency = (Frequency)Storage::loadValueFromUChar(stream);
	m_nextDate.Load(stream, version);
	
	m_type = (Transaction::Type)Storage::loadValueFromUChar(stream);
	m_constraint = (Constraint)Storage::loadValueFromUChar(stream);
}

void ScheduledTransaction::Store(std::fstream &stream) const
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 256 accounts...
	Storage::storeValueToUChar(m_account, stream);
	
	std::bitset<8> localset;	
	localset[0] = m_enabled;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	Storage::storeUChar(cBitset, stream);
	
	Storage::StoreString(m_payee, stream);
	m_amount.Store(stream);
	Storage::StoreString(m_category, stream);
	Storage::StoreString(m_description, stream);
	
	Storage::storeValueToUChar(m_frequency, stream);
	m_nextDate.Store(stream);
	
	Storage::storeValueToUChar(m_type, stream);
	Storage::storeValueToUChar(m_constraint, stream);
}

void ScheduledTransaction::AdvanceNextDate()
{
	switch (m_frequency)
	{
		case Weekly:
			m_nextDate.IncrementDays(7);
			break;
		case TwoWeeks:
			m_nextDate.IncrementDays(14);
			break;
		case FourWeeks:
			m_nextDate.IncrementDays(28);
			break;
		case Monthly:
			m_nextDate.IncrementMonths(1);
			break;
		case TwoMonths:
			m_nextDate.IncrementMonths(2);
			break;
		case Quarterly:
			m_nextDate.IncrementMonths(3);
			break;
		case Annually:
			m_nextDate.IncrementMonths(12);
			break;
	}
	
	if (m_constraint == ExactOrNextWorkingDay)
	{
		m_nextDate.AdvanceToNextWorkingDay();
	}
	else if (m_constraint == LastWorkingDayOfMonth)
	{
		m_nextDate.AdvanceToLastWorkingDayOfMonth();
	}
}
