/*
 *  date.h
 *  money
 *
 *  Created by Peter Pearson on 06/10/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <sstream>
#include <ctime>
#include <fstream>

class Date
{
public:
	Date();
	Date(const Date &rhs);
	Date(time_t Time);
	Date(int Day, int Month, int Year = -1);

	const Date& operator=(const Date& rhs);
	const Date& operator=(time_t Time);
	
	void Now();
	
	void setDate(int Day, int Month, int Year = -1);
	void setYear(int Year);
	void setMonth(int Month);
	void setDay(int Day);

	void setSeparator(char cSep) { m_Separator = cSep; }
	
	int Year() const { return m_Year; }
	int Month() const { return m_Month; }
	int Day() const { return m_Day; }

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

	std::string FormattedDate(int Format) const;

	time_t GetDate() const;
	
private:
	time_t m_Time;

	int m_Year;
	int m_Month;
	int m_Day;

	char m_Separator;
};
