/*
 *  date.cpp
 *  money
 *
 *  Created by Peter Pearson on 06/10/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "datetime.h"

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

	m_Time = mktime(&time);
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

void Date::Load(std::fstream &stream)
{
	stream.read((char *) &m_Time, sizeof(long));
	SetVarsFromTime();
}

void Date::Store(std::fstream &stream)
{
	stream.write((char *) &m_Time, sizeof(long));
}

std::ostream & operator <<( std::ostream & os, const Date & d)
{
	os << d.FormattedDate(0);
	return os;
}

std::string Date::FormattedDate(int Format) const
{
	std::stringstream sDate;

	sDate << m_Day << m_Separator << m_Month << m_Separator << m_Year;

	sDate << std::ends;

	return sDate.str();
}

time_t Date::GetDate() const
{
	return m_Time;
}
