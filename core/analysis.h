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

#include <vector>
#include <map>
#include <set>

#include "graph.h"
#include "fixed.h"
#include "account.h"

class PieChartItem
{
public:
	PieChartItem(const std::string& title, double angle, fixed amount);
	
	void setTitle(const std::string& title) { m_title = title; }
	void setAngle(double angle) { m_angle = angle; }
	void setAmount(fixed amount) { m_amount = amount; }
	
	static bool PieChartSortAngle(const PieChartItem& v1, const PieChartItem& v2)
	{
		return v1.m_angle < v2.m_angle;
	}
	
	static bool PieChartSortTitle(const PieChartItem& v1, const PieChartItem& v2)
	{
		return v1.m_title < v2.m_title;
	}
	
	const std::string&	getTitle() const { return m_title; }
	double				getAngle() const { return m_angle; }
	fixed				getAmount() const { return m_amount; }
	
protected:
	std::string	m_title;
	double		m_angle;
	fixed		m_amount;
};

class AreaChartItem
{
public:
	AreaChartItem(const std::string& title);
	
	void setTitle(const std::string& title) { m_title = title; }
	void addAmountToValue(fixed amount);
	
	bool operator<(const AreaChartItem &rhs) const
	{
		return rhs.m_activeEntries < this->m_activeEntries;
	}
	
	const std::string&	getTitle() const { return m_title; }
	unsigned int		getNumItems() const { return m_amounts.size(); }
	fixed				getItemAmount(int item) const { return m_amounts.at(item); }
	fixed				getMaxValue() const { return m_maxValue; }
	
	void				combineItem(AreaChartItem &item);
	
protected:
	std::string 		m_title;
	std::vector<fixed> 	m_amounts;
	fixed 				m_maxValue;
	bool 				m_blank;
	int 				m_activeEntries;
};

struct OverviewChartItem
{
	OverviewChartItem() { } // only needed for std::map[] if the values isn't found
	OverviewChartItem(MonthYear &date) : m_date(date) { }
	
	void addIncome(const fixed &income)
	{
		m_income += income;
	}
	void addOutgoings(const fixed &outgoings)
	{
		m_outgoings += outgoings;
	}
		
	MonthYear 	m_date;
	fixed 		m_income;
	fixed 		m_outgoings;
};

struct PieChartCriteria
{
	enum PieChartSort
	{
		PieChartSortAngle,
		PieChartSortTitle
	};
	
	PieChartCriteria(const Account* pAccount, Date& startDate, Date& endDate,
					 bool ignoreTransfers, int groupSmaller, std::string& groupSmallerName, PieChartSort eSort) :
		m_pAccount(pAccount), m_startDate(startDate), m_endDate(endDate), m_ignoreTransfers(ignoreTransfers),
		m_groupSmaller(groupSmaller), m_groupSmallerName(groupSmallerName), m_eSort(eSort)
	{
	}
	
	const Account*				m_pAccount;
	Date						m_startDate;
	Date						m_endDate;
	bool 						m_ignoreTransfers;
	int 						m_groupSmaller;
	std::string					m_groupSmallerName;
	PieChartSort 				m_eSort;
	Graph::ItemsType 			m_itemsType;
	std::set<std::string> 		m_aItems;
};

struct PieChartResults
{
	std::vector<PieChartItem>	m_aValues;
	fixed						m_overallTotal;
};

bool buildPieChartItems(const PieChartCriteria& criteria, PieChartResults& results, bool expense, bool categories);

void copyPieItemsToVector(std::map<std::string, fixed>& aMap, const PieChartCriteria& criteria, PieChartResults& results);

struct AreaChartCriteria
{
	AreaChartCriteria(const Account* pAccount, const Date& startDate, const Date& endDate, bool ignoreTransfers,
		int groupSmaller, std::string &groupSmallerName) :
		m_pAccount(pAccount), m_startDate(startDate), m_endDate(endDate), m_ignoreTransfers(ignoreTransfers),
		m_groupSmaller(groupSmaller), m_groupSmallerName(groupSmallerName)
	{
	}
	
	const Account*				m_pAccount;
	
	Date						m_startDate;
	Date						m_endDate;
	bool 						m_ignoreTransfers;
	int 						m_groupSmaller;
	std::string					m_groupSmallerName;
	Graph::ItemsType 			m_itemsType;
	std::set<std::string> 		m_aItems;
};

struct AreaChartResults
{
	std::vector<AreaChartItem>	m_aValues;
	std::vector<MonthYear>		m_aDates;
	fixed						m_overallMax;
};

bool buildAreaChartItems(const AreaChartCriteria& criteria, AreaChartResults& results, bool expense, bool categories);

void copyAreaItemsToVector(std::map<std::string, std::map<MonthYear, fixed> >& aMap, std::map<MonthYear, fixed>& aDateTotals,
						   const AreaChartCriteria& criteria, AreaChartResults& results);

struct OverviewChartCriteria
{
	OverviewChartCriteria(const Account* pAccount, const Date &startDate, const Date &endDate, bool ignoreTransfers, fixed &overallMax) :
	m_pAccount(pAccount), m_startDate(startDate), m_endDate(endDate), m_ignoreTransfers(ignoreTransfers), m_overallMax(overallMax)
	{
	}
	
	const Account*			m_pAccount;
	Date					m_startDate;
	Date					m_endDate;
	bool 					m_ignoreTransfers;
	
	fixed&					m_overallMax;
};

bool shouldItemBeIncluded(Graph::ItemsType eType, const std::set<std::string>& aItems, const std::string& item);

bool buildOverviewChartItems(const OverviewChartCriteria& criteria, std::vector<OverviewChartItem>& aItems);
