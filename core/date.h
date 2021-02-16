/*
 * Stash:  A Personal Finance app (core).
 * Copyright (C) 2009-2021 Peter Pearson
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

#ifndef DATETIME_H
#define DATETIME_H

#include <iostream>
#include <sstream>
#include <ctime>
#include <fstream>
#include <stdint.h>

class Date
{
public:
	enum DateStringFormat
	{
		UK,
		US,
		OFX
	};

	Date();
	Date(const Date &rhs);
	Date(time_t Time);
	Date(unsigned int Day, unsigned int Month, unsigned int Year);
	Date(const std::string& date, char cSep, DateStringFormat dateFormat = UK);

	Date& operator=(const Date& rhs);
	Date& operator=(time_t Time);
	
	void IncrementDays(int days);
	void IncrementMonths(int months);
	
	void DecrementDays(int days);
	void DecrementMonths(int months);
	
	void AdvanceToNextWorkingDay();
	void AdvanceToLastWorkingDayOfMonth();
	
	void Now();
	
	void setDate(unsigned int Day, unsigned int Month, unsigned int Year);
	void setDate(std::string date, char cSep, DateStringFormat dateFormat = UK);
	void setYear(unsigned int Year);
	void setMonth(unsigned int Month);
	void setDay(unsigned int Day);

	void setSeparator(char cSep) { m_Separator = cSep; }
	
	unsigned int getYear() const { return (unsigned int)m_Year; }
	unsigned int getMonth() const { return (unsigned int)m_Month; }
	unsigned int getDay() const { return (unsigned int)m_Day; }
	
	bool isLeapYear() const;

	bool operator==(const Date& date) const { return m_Time == date.m_Time; }
	bool operator!=(const Date& date) const { return m_Time != date.m_Time; }
	bool operator>(const Date& date) const { return m_Time > date.m_Time; }
	bool operator<(const Date& date) const { return m_Time < date.m_Time; }
	bool operator>=(const Date& date) const { return m_Time >= date.m_Time; }
	bool operator<=(const Date& date) const { return m_Time <= date.m_Time; }

	void SetVarsFromTime();
	void SetTimeFromVars();

	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream) const;

	// these versions are old ones
	void LoadOld(std::fstream &stream, int version);
	void StoreOld(std::fstream &stream) const;

	friend std::ostream& operator<< (std::ostream & os, const Date & d);

	std::string FormattedDate(DateStringFormat format) const;

	time_t GetDate() const;
	
private:
	time_t		m_Time;

	uint16_t	m_Year;
	uint8_t		m_Month;
	uint8_t		m_Day;
	uint8_t		m_DayOfWeek;

	char		m_Separator;
};

class MonthYear
{
public:
	MonthYear() : m_month(-1), m_year(0) { } // needed for std::map[]
	MonthYear(int month, int year);
	
	int getMonth() const { return m_month; }
	int getYear() const { return m_year; }
	
	void increment1();
	
	bool operator==(MonthYear my) const { return m_month == my.m_month && m_year == my.m_year; }
	bool operator!=(MonthYear my) const { return m_month != my.m_month || m_year != my.m_year; }
	bool operator<(MonthYear my) const { return m_year < my.m_year || (m_month < my.m_month && m_year == my.m_year); }
	
protected:
	int m_month;
	int m_year;	
};

#endif
