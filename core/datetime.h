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

#ifndef DATETIME_H
#define DATETIME_H

#include <iostream>
#include <sstream>
#include <ctime>
#include <fstream>

enum DateStringFormat
{
	UK,
	US
};

#define SECONDS_IN_DAY 86400

class Date
{
public:
	Date();
	Date(const Date &rhs);
	Date(time_t Time);
	Date(int Day, int Month, int Year = -1);
	Date(std::string date, char cSep, DateStringFormat dateFormat = UK);

	const Date& operator=(const Date& rhs);
	const Date& operator=(time_t Time);
	
	void IncrementDays(int days);
	void IncrementMonths(int months);
	
	void DecrementDays(int days);
	void DecrementMonths(int months);
	
	void AdvanceToNextWorkingDay();
	
	void Now();
	
	void setDate(int Day, int Month, int Year = -1);
	void setDate(std::string date, char cSep, DateStringFormat dateFormat = UK);
	void setYear(int Year);
	void setMonth(int Month);
	void setDay(int Day);

	void setSeparator(char cSep) { m_Separator = cSep; }
	
	int getYear() const { return m_Year; }
	int getMonth() const { return m_Month; }
	int getDay() const { return m_Day; }
	
	bool isLeapYear() const;

	bool operator==(Date date) const { return m_Time == date.m_Time; }
	bool operator!=(Date date) const { return m_Time != date.m_Time; }
	bool operator>(Date date) const { return m_Time > date.m_Time; }
	bool operator<(Date date) const { return m_Time < date.m_Time; }
	bool operator>=(Date date) const { return m_Time >= date.m_Time; }
	bool operator<=(Date date) const { return m_Time <= date.m_Time; }

	void SetVarsFromTime();
	void SetTimeFromVars();

	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);

	friend std::ostream& operator<< (std::ostream & os, const Date & d);

	std::string FormattedDate(DateStringFormat format) const;

	time_t GetDate() const;
	
private:
	time_t m_Time;

	int m_Year;
	int m_Month;
	int m_Day;
	int m_DayOfWeek;

	char m_Separator;
};

class MonthYear
{
public:
	MonthYear(int month, int year);
	
	int getMonth() { return m_month; }
	int getYear() { return m_year; }
	
	bool operator==(MonthYear my) const { return m_month == my.m_month && m_year == my.m_year; }
	bool operator!=(MonthYear my) const { return m_month != my.m_month || m_year != my.m_year; }
	bool operator<(MonthYear my) const { return m_year < my.m_year || (m_month < my.m_month && m_year == my.m_year); }
protected:
	int m_month;
	int m_year;	
	
};

#endif
