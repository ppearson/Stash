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

#include "string.h"
#include <bitset>
#include "scheduled_transaction.h"

ScheduledTransaction::ScheduledTransaction() : m_enabled(true), m_frequency(Weekly), m_account(0), m_type(None), m_constraint(ExactDate)
{
	m_nextDate.Now();
}

void ScheduledTransaction::Load(std::fstream &stream, int version)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 512 accounts...
	stream.read((char *) &m_account, sizeof(unsigned char));
	
	if (version == 0)
	{
		m_enabled = true;
	}
	else
	{
		unsigned char cBitset = 0;
		stream.read((char *) &cBitset, sizeof(unsigned char));
	
		std::bitset<8> localset(static_cast<unsigned long>(cBitset));
	
		m_enabled = localset[0];
	}
	
	LoadString(m_payee, stream);
	m_amount.Load(stream, version);
	LoadString(m_category, stream);
	LoadString(m_description, stream);
	
	stream.read((char *) &m_frequency, sizeof(unsigned char));
	m_nextDate.Load(stream, version);
	
	stream.read((char *) &m_type, sizeof(unsigned char));
	stream.read((char *) &m_constraint, sizeof(unsigned char));
}

void ScheduledTransaction::Store(std::fstream &stream)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 512 accounts...
	stream.write((char *) &m_account, sizeof(unsigned char));
	
	std::bitset<8> localset;	
	localset[0] = m_enabled;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	stream.write((char *) &cBitset, sizeof(unsigned char));
	
	StoreString(m_payee, stream);
	m_amount.Store(stream);
	StoreString(m_category, stream);
	StoreString(m_description, stream);
	
	stream.write((char *) &m_frequency, sizeof(unsigned char));
	m_nextDate.Store(stream);
	
	stream.write((char *) &m_type, sizeof(unsigned char));
	stream.write((char *) &m_constraint, sizeof(unsigned char));
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
}