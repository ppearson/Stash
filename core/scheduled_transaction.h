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

enum DateFallsOn
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
	std::string getPayee() { return m_payee; }
	fixed getAmount() { return m_amount; }
	std::string getCategory() { return m_category; }
	std::string getDescription() { return m_description; }
	Frequency getFrequency() { return m_frequency; }
	Date getNextDate() { return m_nextDate; }
	const Date &getNextDate2() { return m_nextDate; }
	TransactionType getType() { return m_type; }
	
	void setAccount(int account) { m_account = account; }
	void setPayee(std::string payee) { m_payee = payee; }
	void setAmount(fixed amount) { m_amount = amount; }
	void setCategory(std::string category) { m_category = category; }
	void setDescription(std::string description) { m_description = description; }
	void setFrequency(Frequency frequency) { m_frequency = frequency; }
	void setNextDate(Date nextDate) { m_nextDate = nextDate; }
	void setType(TransactionType type) { m_type = type; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);
	
	void AdvanceNextDate();
	
protected:
	int m_account;
	std::string m_payee;
	fixed m_amount;
	std::string m_category;
	std::string m_description;	
	
	Frequency m_frequency;
	Date m_nextDate;
	TransactionType m_type;
	DateFallsOn m_dateFallsOn;
	
};

#endif
