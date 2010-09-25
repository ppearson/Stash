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

#include <vector>
#include <map>
#include <set>

#include "graph.h"
#include "fixed.h"
#include "account.h"

enum PieChartSort
{
	PieChartSortAngle,
	PieChartSortTitle
};

class PieChartItem
{
public:
	PieChartItem(std::string title, double angle, fixed amount);
	~PieChartItem() { };
	
	void setTitle(std::string title) { m_title = title; }
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
	
	std::string		getTitle() { return m_title; }
	double			getAngle() { return m_angle; }
	fixed			getAmount() { return m_amount; }	
	
protected:
	std::string	m_title;
	double		m_angle;
	fixed		m_amount;
};

class AreaChartItem
{
public:
	AreaChartItem(std::string title);
	~AreaChartItem() { }
	
	void setTitle(std::string title) { m_title = title; }
	void addAmountToValue(fixed amount);
	
	bool operator<(const AreaChartItem &rhs) const
	{
		return rhs.m_activeEntries < this->m_activeEntries;
	}
	
	std::string		getTitle() { return m_title; }
	int				getNumItems() { return m_amounts.size(); }
	fixed			getItemAmount(int item) { return m_amounts.at(item); }
	fixed			getMaxValue() { return m_maxValue; }
	
	void			combineItem(AreaChartItem &item);
	
protected:
	std::string m_title;
	std::vector<fixed> m_amounts;
	fixed m_maxValue;
	bool m_blank;
	int m_activeEntries;
};

struct OverviewChartItem
{
	OverviewChartItem() { } // only needed for std::map[] if the values isn't found
	OverviewChartItem(MonthYear &date) : m_date(date) { }
	~OverviewChartItem() { }
	
	void addIncome(fixed &income) { m_income += income; }
	void addOutgoings(fixed &outgoings) { m_outgoings += outgoings; }
		
	MonthYear m_date;
	fixed m_income;
	fixed m_outgoings;
};

struct PieChartCriteria
{
	PieChartCriteria(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal,
					 bool ignoreTransfers, int groupSmaller, std::string &groupSmallerName, PieChartSort eSort) :
		m_pAccount(pAccount), m_aValues(aValues), m_startDate(startDate), m_endDate(endDate), m_overallTotal(overallTotal), m_ignoreTransfers(ignoreTransfers),
		m_groupSmaller(groupSmaller), m_groupSmallerName(groupSmallerName), m_eSort(eSort)
	{
	}
	
	Account *m_pAccount;
	std::vector<PieChartItem> &m_aValues;
	Date &m_startDate;
	Date &m_endDate;
	fixed &m_overallTotal;
	bool m_ignoreTransfers;
	int m_groupSmaller;
	std::string &m_groupSmallerName;
	PieChartSort m_eSort;
	GraphItemsType m_itemsType;
	std::set<std::string> m_aItems;
};

bool buildPieChartItems(Graph *pGraph, PieChartCriteria &criteria, bool expense, bool categories);

void copyPieItemsToVector(std::map<std::string, fixed> &aMap, PieChartCriteria &criteria);

struct AreaChartCriteria
{
	AreaChartCriteria(Account *pAccount, std::vector<AreaChartItem> &aValues, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate, fixed &overallMax, bool ignoreTransfers,
					int groupSmaller, std::string &groupSmallerName) :
		m_pAccount(pAccount), m_aValues(aValues), m_aDates(aDates), m_startDate(startDate), m_endDate(endDate), m_overallMax(overallMax), m_ignoreTransfers(ignoreTransfers),
		m_groupSmaller(groupSmaller), m_groupSmallerName(groupSmallerName)
	{
	}
	
	Account *m_pAccount;
	std::vector<AreaChartItem> &m_aValues;
	std::vector<MonthYear> &m_aDates;
	Date &m_startDate;
	Date &m_endDate;
	fixed &m_overallMax;
	bool m_ignoreTransfers;
	int m_groupSmaller;
	std::string &m_groupSmallerName;
	GraphItemsType m_itemsType;
	std::set<std::string> m_aItems;
};

struct OverviewChartCriteria
{
	OverviewChartCriteria(Account *pAccount, Date &startDate, Date &endDate, fixed &overallMax, bool ignoreTransfers) :
		m_pAccount(pAccount), m_startDate(startDate), m_endDate(endDate), m_ignoreTransfers(ignoreTransfers), m_overallMax(overallMax)
	{
	}
	
	Account *m_pAccount;
	Date &m_startDate;
	Date &m_endDate;
	bool m_ignoreTransfers;
	fixed &m_overallMax;
};
	

bool buildAreaChartItems(Graph *pGraph, AreaChartCriteria &criteria, bool expense, bool categories);

void copyAreaItemsToVector(std::map<std::string, std::map< MonthYear, fixed > > &aMap, std::map<MonthYear, fixed> &aDateTotals, AreaChartCriteria &criteria);

bool shouldItemBeIncluded(GraphItemsType eType, std::set<std::string> &aItems, std::string &item);

bool buildOverviewChartItems(OverviewChartCriteria &criteria, std::vector<OverviewChartItem> &aItems);
