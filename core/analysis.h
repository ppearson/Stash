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

#include "fixed.h"
#include "Account.h"

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

bool buildPieChartItemsForExpenseCategories(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal,
											bool ignoreTransfers, int groupSmaller, std::string &groupSmallerName, PieChartSort eSort);
bool buildPieChartItemsForExpensePayees(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal,
										bool ignoreTransfers, int groupSmaller, std::string &groupSmallerName, PieChartSort eSort);
bool buildPieChartItemsForDepositCategories(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal,
											bool ignoreTransfers, int groupSmaller, std::string &groupSmallerName, PieChartSort eSort);
bool buildPieChartItemsForDepositPayees(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal,
										bool ignoreTransfers, int groupSmaller, std::string &groupSmallerName, PieChartSort eSort);

bool buildAreaChartItemsForExpenseCategories(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate, 
											 fixed &overallMax, bool ignoreTransfers);
bool buildAreaChartItemsForExpensePayees(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate, 
											 fixed &overallMax, bool ignoreTransfers);
bool buildAreaChartItemsForDepositCategories(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate, 
											 fixed &overallMax, bool ignoreTransfers);
bool buildAreaChartItemsForDepositPayees(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate, 
											 fixed &overallMax, bool ignoreTransfers);

void copyPieItemsToVector(std::map<std::string, fixed> &aMap, std::vector<PieChartItem> &aVector, fixed &overallTotal, int groupSmaller, std::string &groupSmallerName,
									PieChartSort eSort);
void copyAreaItemsToVector(std::map<std::string, std::map< MonthYear, fixed > > &aMap, std::map<MonthYear, fixed> &aDateTotals, std::vector<AreaChartItem> &aItems,
						   std::vector<MonthYear> &aDates, fixed &overallMax);
