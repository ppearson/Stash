#include "string.h"
#include "scheduled_transaction.h"

ScheduledTransaction::ScheduledTransaction() : m_frequency(Weekly), m_account(0), m_type(None), m_dateFallsOn(ExactDate)
{
	m_nextDate.Now();
}

void ScheduledTransaction::Load(std::fstream &stream, int version)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 512 accounts...
	stream.read((char *) &m_account, sizeof(unsigned char));
	LoadString(m_payee, stream);
	m_amount.Load(stream, version);
	LoadString(m_category, stream);
	LoadString(m_description, stream);
	
	stream.read((char *) &m_frequency, sizeof(unsigned char));
	m_nextDate.Load(stream, version);
	
	stream.read((char *) &m_type, sizeof(unsigned char));
	stream.read((char *) &m_dateFallsOn, sizeof(unsigned char));
}

void ScheduledTransaction::Store(std::fstream &stream)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 512 accounts...
	stream.write((char *) &m_account, sizeof(unsigned char));
	StoreString(m_payee, stream);
	m_amount.Store(stream);
	StoreString(m_category, stream);
	StoreString(m_description, stream);
	
	stream.write((char *) &m_frequency, sizeof(unsigned char));
	m_nextDate.Store(stream);
	
	stream.write((char *) &m_type, sizeof(unsigned char));
	stream.write((char *) &m_dateFallsOn, sizeof(unsigned char));
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
}