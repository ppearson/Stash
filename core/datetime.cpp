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

#include <algorithm>
#include <iomanip>

#include "datetime.h"

static int daysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

Date::Date() : m_Separator('/')
{
	Now();
}

Date::Date(const Date &rhs) : m_Separator('/')
{
	m_Day = rhs.m_Day;
	m_Month = rhs.m_Month;
	m_Year = rhs.m_Year;

	SetTimeFromVars();
}

Date::Date(time_t Time)
{
	m_Time = Time;

	SetVarsFromTime();
}

Date::Date(int Day, int Month, int Year) :
	m_Day(Day), m_Month(Month), m_Year(Year), m_Separator('/')
{
	SetTimeFromVars();
}

Date::Date(std::string date, char cSep, DateStringFormat dateFormat) : m_Separator(cSep)
{
	setDate(date, cSep, dateFormat);
}

const Date& Date::operator=(const Date& rhs)
{
	m_Time = rhs.m_Time;
	SetVarsFromTime();
	return *this;
}

const Date& Date::operator=(time_t Time)
{
	m_Time = Time;
	SetVarsFromTime();
	return *this;
}

void Date::IncrementDays(int days)
{
	m_Time += (SECONDS_IN_DAY * days);
	
	SetVarsFromTime();
}

void Date::IncrementMonths(int months)
{
	for (int i = 0; i < months; i++)
	{
		IncrementDays(daysInMonth[m_Month - 1]);		
	}
	
	SetVarsFromTime();
}

void Date::DecrementDays(int days)
{
	m_Time -= (SECONDS_IN_DAY * days);
	
	SetVarsFromTime();
}

void Date::DecrementMonths(int months)
{
	for (int i = 0; i < months; i++)
	{
		DecrementDays(daysInMonth[m_Month - 1]);		
	}
	
	SetVarsFromTime();
}

void Date::Now()
{
	time(&m_Time);

	SetVarsFromTime();
}

void Date::SetVarsFromTime()
{
	struct tm *time;
	time = localtime(&m_Time);

	if (time)
	{
		m_Month = time->tm_mon + 1;
		m_Day = time->tm_mday;
		m_Year = time->tm_year + 1900;
	}
}

void Date::SetTimeFromVars()
{
	struct tm time;
	
	time.tm_sec = 0;
	time.tm_min = 0;
	time.tm_hour = 0;

	time.tm_mday = m_Day;
	time.tm_mon = m_Month - 1;
	time.tm_year = m_Year - 1900;
	time.tm_isdst = 0;

	m_Time = mktime(&time);
}

void Date::setDate(std::string date, char cSep, DateStringFormat dateFomat)
{
	if (date.find(cSep) == -1)
		return;

	std::replace(date.begin(), date.end(), cSep, ' ');

	std::stringstream strm(date);
	std::string strDay;
	std::string strMonth;
	std::string strYear;
	
	if (dateFomat == UK)
	{
		strm >> strDay;
		strm >> strMonth;
	}
	else
	{
		strm >> strMonth;
		strm >> strDay;
	}

	strm >> strYear;

	int nDay = atoi(strDay.c_str());
	int nMonth = atoi(strMonth.c_str());
	int nYear = atoi(strYear.c_str());
	
	if (strYear.length() == 2)
	{
		if (nYear > 85)
		{
			nYear += 1900;
		}
		else
		{
			nYear += 2000;
		}
	}

	setSeparator(cSep);
	setDate(nDay, nMonth, nYear);
}

void Date::setYear(int Year)
{
	m_Year = Year;
	SetTimeFromVars();
}

void Date::setMonth(int Month)
{
	m_Month = Month;
	SetTimeFromVars();
}

void Date::setDay(int Day)
{
	m_Day = Day;
	SetTimeFromVars();
}

void Date::setDate(int Day, int Month, int Year)
{
	m_Day = Day;
	m_Month = Month;
	m_Year = Year;

	SetTimeFromVars();
}

void Date::Load(std::fstream &stream, int version)
{
	// time_t is different sizes on 32/64 bit systems
	long long tempTime = 0;
	stream.read((char *) &tempTime, sizeof(long long));
	m_Time = static_cast<time_t>(tempTime);
	SetVarsFromTime();
}

void Date::Store(std::fstream &stream)
{
	// time_t is different sizes on 32/64 bit systems
	long long tempTime = static_cast<long long>(m_Time);
	stream.write((char *) &tempTime, sizeof(long long));
}

std::ostream & operator <<( std::ostream & os, const Date & d)
{
	os << d.FormattedDate(UK);
	return os;
}

std::string Date::FormattedDate(DateStringFormat format) const
{
	std::stringstream sDate;

	if (format == UK)
	{
		sDate << std::setw(2) << std::setfill('0') << m_Day;
		sDate << m_Separator;
		sDate << std::setw(2) << std::setfill('0') << m_Month;
		sDate << m_Separator << m_Year;
	}
	else if (format == US)
	{
		sDate << std::setw(2) << std::setfill('0') << m_Month;
		sDate << m_Separator;
		sDate << std::setw(2) << std::setfill('0') << m_Day;
		sDate << m_Separator << m_Year;
	}

	sDate << std::ends;

	return sDate.str();
}

time_t Date::GetDate() const
{
	return m_Time;
}
