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

#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include "analysis.h"
#include "transaction.h"

PieChartItem::PieChartItem(std::string title, double angle, fixed amount) : m_title(title), m_angle(angle), m_amount(amount)
{

}

AreaChartItem::AreaChartItem(std::string title) : m_title(title), m_maxValue(0.0), m_blank(true), m_activeEntries(0)
{

}

void AreaChartItem::addAmountToValue(fixed amount)
{	
	m_blank = false;
	
	m_amounts.push_back(amount);
	
	if (!amount.IsZero())
		m_activeEntries++;
	
	if (m_maxValue < amount)
		m_maxValue = amount;
}

void AreaChartItem::combineItem(AreaChartItem &item)
{	
	std::vector<fixed>::iterator itemIt = item.m_amounts.begin();
	std::vector<fixed>::iterator itemItEnd = item.m_amounts.end();
	
	if (m_blank)
	{		
		for (; itemIt != itemItEnd; ++itemIt)
		{
			fixed &itemValue = (*itemIt);
			
			if (!itemValue.IsZero())
				m_activeEntries++;
			
			m_amounts.push_back(itemValue);
		}
		
		m_blank = false;
	}
	else
	{
		std::vector<fixed>::iterator thisIt = m_amounts.begin();
		std::vector<fixed>::iterator thisItEnd = m_amounts.end();
		
		for (; thisIt != thisItEnd && itemIt != itemItEnd; ++thisIt, ++itemIt)
		{
			fixed &thisValue = (*thisIt);
			
			fixed &itemValue = (*itemIt);
			
			thisValue += itemValue;
		}
		
		if (m_activeEntries < item.m_activeEntries)
			m_activeEntries = item.m_activeEntries;
	}
}

bool buildPieChartItems(Graph *pGraph, PieChartCriteria &criteria, bool expense, bool categories)
{
	if (!criteria.m_pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::const_iterator it = criteria.m_pAccount->begin();
	std::vector<Transaction>::const_iterator itEnd = criteria.m_pAccount->end();

	for (; it != itEnd; ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if (expense && (*it).getAmount().IsPositive() ||
			!expense && !(*it).getAmount().IsPositive())
		{
			continue;
		}
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				const SplitTransaction &split = (*it).getSplit(i);
				
				std::string item;
				if (categories)
					item = split.getCategory();
				else
					item = split.getPayee();
				
				if (!shouldItemBeIncluded(criteria.m_itemsType, criteria.m_aItems, item))
					continue;
				
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itFind = aMap.find(item);
				
				if (itFind == aMap.end())
				{
					aMap[item] = amount;
				}
				else
				{
					fixed &catTotal = (*itFind).second;
					
					catTotal += amount;			
				}
				
				criteria.m_overallTotal += amount;
			}			
		}
		else
		{
			std::string item;
			if (categories)
				item = (*it).getCategory();
			else
				item = (*it).getPayee();
			
			if (!shouldItemBeIncluded(criteria.m_itemsType, criteria.m_aItems, item))
				continue;
			
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itFind = aMap.find(item);
			
			if (itFind == aMap.end())
			{
				aMap[item] = amount;
			}
			else
			{
				fixed &catTotal = (*itFind).second;
				
				catTotal += amount;			
			}
			
			criteria.m_overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, criteria);

	return true;
}

bool buildAreaChartItems(Graph *pGraph, AreaChartCriteria &criteria, bool expense, bool categories)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::const_iterator it = criteria.m_pAccount->begin();
	std::vector<Transaction>::const_iterator itEnd = criteria.m_pAccount->end();
	
	for (; it != itEnd; ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if (expense && (*it).getAmount().IsPositive() || !expense && !(*it).getAmount().IsPositive())
		{
			continue;
		}
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
	
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				const SplitTransaction &split = (*it).getSplit(i);
				
				std::string item;
				if (categories)
					item = split.getCategory();
				else
					item = split.getPayee();
				
				if (!shouldItemBeIncluded(criteria.m_itemsType, criteria.m_aItems, item))
					continue;
				
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itDateTotal = aDateTotals.find(my);
				
				if (itDateTotal == aDateTotals.end())
				{
					aDateTotals[my] = amount;
				}
				else
				{
					fixed &dateTotal = (*itDateTotal).second;
					
					dateTotal += amount;
				}
				
				itItemFind = aItemMap.find(item);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[item] = dateMap;
				}
				else
				{
					std::map<MonthYear, fixed> &dateMap = (*itItemFind).second;
					
					std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(my);
					
					if (itDateFind == dateMap.end())
					{
						dateMap[my] = amount;						
					}
					else
					{
						fixed &itemTotal = (*itDateFind).second;
						
						itemTotal += amount;
					}		
				}
			}			
		}
		else
		{
			std::string item;
			if (categories)
				item = (*it).getCategory();
			else
				item = (*it).getPayee();
			
			if (!shouldItemBeIncluded(criteria.m_itemsType, criteria.m_aItems, item))
				continue;
			
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itDateTotal = aDateTotals.find(my);
			
			if (itDateTotal == aDateTotals.end())
			{
				aDateTotals[my] = amount;
			}
			else
			{
				fixed &dateTotal = (*itDateTotal).second;
				
				dateTotal += amount;
			}
			
			itItemFind = aItemMap.find(item);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[item] = dateMap;
			}
			else
			{
				std::map<MonthYear, fixed> &dateMap = (*itItemFind).second;
				
				std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(my);
				
				if (itDateFind == dateMap.end())
				{
					dateMap[my] = amount;				
				}
				else
				{
					fixed &itemTotal = (*itDateFind).second;
					
					itemTotal += amount;
				}
			}
		}
	}

	copyAreaItemsToVector(aItemMap, aDateTotals, criteria);
	
	return true;
}

void copyPieItemsToVector(std::map<std::string, fixed> &aMap, PieChartCriteria &criteria)
{
	double dOverallTotal = criteria.m_overallTotal.ToDouble();
	
	fixed leftovers = 0.0;
	
	// copy values to the vector
	
	std::map<std::string, fixed>::iterator itMap = aMap.begin();
	std::map<std::string, fixed>::iterator itMapEnd = aMap.end();
	
	double dGroupSmaller = static_cast<double>(criteria.m_groupSmaller);
	
	for (; itMap != itMapEnd; ++itMap)
	{
		std::string title = (*itMap).first;
		
		fixed amount = (*itMap).second;
		
		if (title.empty() || title == criteria.m_groupSmallerName)
		{
			leftovers += amount;
			continue;
		}
		
		double dPieAngle = (amount.ToDouble() / dOverallTotal) * 360.0;
		
		if (criteria.m_groupSmaller == -1 || dPieAngle > dGroupSmaller)
		{
			PieChartItem newGraphValue(title, dPieAngle, amount);
			
			criteria.m_aValues.push_back(newGraphValue);
		}
		else
		{
			leftovers += amount;
		}
	}
	
	// sort the values
	
	if (criteria.m_eSort == PieChartSortTitle)
	{
		std::sort(criteria.m_aValues.begin(), criteria.m_aValues.end(), PieChartItem::PieChartSortTitle);
	}
	else
	{
		std::sort(criteria.m_aValues.begin(), criteria.m_aValues.end(), PieChartItem::PieChartSortAngle);
	}
	
	// add other category with any leftovers that are too small to bother showing
	
	if (!leftovers.IsZero())
	{
		double dPieAngle = (leftovers.ToDouble() / dOverallTotal) * 360.0;
		
		PieChartItem newGraphValue(criteria.m_groupSmallerName, dPieAngle, leftovers);
		
		criteria.m_aValues.push_back(newGraphValue);
	}	
}

void copyAreaItemsToVector(std::map<std::string, std::map< MonthYear, fixed > > &aMap, std::map<MonthYear, fixed> &aDateTotals, AreaChartCriteria &criteria)
{
	// to make sure we don't have holes in date ranges, increment through them and add them if they don't exist
	
	if (!aDateTotals.empty())
	{
		std::map<MonthYear, fixed>::iterator itStartDate = aDateTotals.begin();
		std::map<MonthYear, fixed>::iterator itEndDate = aDateTotals.begin();
		
		int size = aDateTotals.size();
		
		for (int i = 1; i < size; i++)
			++itEndDate;
		
		MonthYear startDate = (*itStartDate).first;
		MonthYear endDate = (*itEndDate).first;
		
		MonthYear DateItem = startDate;
		
		while (DateItem < endDate)
		{
			DateItem.increment1();
			
			std::map<MonthYear, fixed>::iterator itDateFind = aDateTotals.find(DateItem);
			
			if (itDateFind == aDateTotals.end())
			{
				aDateTotals[DateItem] = 0.0;
			}			
		}
	}
	
	//	for each item, for each date that exists in the aDates set, create an AreaChartItem for that date and the item
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItem = aMap.begin();
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemEnd = aMap.end();
	
	for (; itItem != itItemEnd; ++itItem)
	{
		std::string title = (*itItem).first;
		
		std::map<MonthYear, fixed>::iterator itDate = aDateTotals.begin();
		std::map<MonthYear, fixed>::iterator itDateEnd = aDateTotals.end();
		
		AreaChartItem newItem(title);
		
		for (; itDate != itDateEnd; ++itDate)
		{
			std::map<MonthYear, fixed> &dateMap = (*itItem).second;
			
			MonthYear myDate = (*itDate).first;
			
			fixed &dateTotal = (*itDate).second;
			
			if (criteria.m_overallMax < dateTotal)
				criteria.m_overallMax = dateTotal;
			
			std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(myDate);
			
			if (itDateFind == dateMap.end())
			{
				newItem.addAmountToValue(0.0);				
			}
			else
			{
				fixed &amount = (*itDateFind).second;
				
				newItem.addAmountToValue(amount);
			}
		}
		
		criteria.m_aValues.push_back(newItem);
	}
	
	if (criteria.m_groupSmaller > 0)
	{
		// now work out what's under the group smaller setting percentage of the max overall value and if the item's less than that, remove it
		
		double dPurgeValue = criteria.m_overallMax.ToDouble();
		double dPercentage = (double)criteria.m_groupSmaller / 100.0;
		
		dPurgeValue *= dPercentage;
		
		bool bAddOther = false;
		AreaChartItem otherItem(criteria.m_groupSmallerName);
		
		std::vector<AreaChartItem>::iterator itPurgeItem = criteria.m_aValues.begin();
		std::vector<AreaChartItem>::iterator itPurgeItemEnd = criteria.m_aValues.end();
		
		while (itPurgeItem != itPurgeItemEnd)
		{
			if ((*itPurgeItem).getMaxValue().ToDouble() < dPurgeValue)
			{
				otherItem.combineItem(*itPurgeItem);
				
				itPurgeItem = criteria.m_aValues.erase(itPurgeItem);
				itPurgeItemEnd = criteria.m_aValues.end();
				
				bAddOther = true;
			}
			else
			{
				++itPurgeItem;
			}
		}
		
		// In case there's an other item already, we need to combine the existing and our new one
		std::vector<AreaChartItem>::iterator itExistingOtherItem = criteria.m_aValues.begin();
		std::vector<AreaChartItem>::iterator itExistingOtherItemEnd = criteria.m_aValues.end();
		
		while (itExistingOtherItem != itExistingOtherItemEnd)
		{
			if ((*itExistingOtherItem).getTitle() == criteria.m_groupSmallerName)
				break;
			
			++itExistingOtherItem;
		}
		
		if (bAddOther)
		{
			if (itExistingOtherItem != criteria.m_aValues.end())
			{
				((*itExistingOtherItem).combineItem(otherItem));
			}
			else
			{
				criteria.m_aValues.push_back(otherItem);
			}
		}
	}
	
	// sort the items so that items with fewer actual values (most likely occasional expenditures) get done last
	// so they won't affect the more regular items, and will stand out more 
	std::sort(criteria.m_aValues.begin(), criteria.m_aValues.end());
		
	// add dateitems to vector
	std::map<MonthYear, fixed>::iterator itDate2 = aDateTotals.begin();
	std::map<MonthYear, fixed>::iterator itDate2End = aDateTotals.end();
	
	for (; itDate2 != itDate2End; ++itDate2)
	{
		MonthYear myDate = (*itDate2).first;
		
		criteria.m_aDates.push_back(myDate);		
	}
}

bool shouldItemBeIncluded(GraphItemsType eType, std::set<std::string> &aItems, std::string &item)
{
	if (eType == AllItems)
		return true;
	
	std::set<std::string>::iterator it = aItems.find(item);
	
	if (eType == AllItemsExceptSpecified)
	{
		if (it == aItems.end())
		{
			return true;
		}
	}
	else if (eType == OnlySpecified)
	{
		if (it != aItems.end())
		{
			return true;
		}
	}
	
	return false;
}

bool buildOverviewChartItems(OverviewChartCriteria &criteria, std::vector<OverviewChartItem> &aItems)
{
	std::map<MonthYear, OverviewChartItem> aBuildItems;
	std::map<MonthYear, OverviewChartItem>::iterator itDateFind = NULL;
	
	bool income = true;
	
	std::vector<Transaction>::const_iterator it = criteria.m_pAccount->begin();
	std::vector<Transaction>::const_iterator itEnd = criteria.m_pAccount->end();
	
	for (; it != itEnd; ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if ((*it).getAmount().IsPositive())
			income = true;
		else
			income = false;
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());
			
		fixed amount = (*it).getAmount();
			
		amount.setPositive();
		
		itDateFind = aBuildItems.find(my);
		
		if (itDateFind == aBuildItems.end())
		{
			OverviewChartItem newItem(my);
			
			if (income)
				newItem.addIncome(amount);
			else
				newItem.addOutgoings(amount);
			
			aBuildItems[my] = newItem;
		}
		else
		{
			OverviewChartItem &item = (*itDateFind).second;
			
			if (income)
				item.addIncome(amount);
			else
				item.addOutgoings(amount);
		}
	}
	
	itDateFind = aBuildItems.begin();
	std::map<MonthYear, OverviewChartItem>::iterator itDateFindEnd = aBuildItems.end();
	
	for (; itDateFind != itDateFindEnd; ++itDateFind)
	{
		if ((*itDateFind).second.m_income > criteria.m_overallMax)
		{
			criteria.m_overallMax = (*itDateFind).second.m_income;
		}
		
		if ((*itDateFind).second.m_outgoings > criteria.m_overallMax)
		{
			criteria.m_overallMax = (*itDateFind).second.m_outgoings;
		}
		
		aItems.push_back((*itDateFind).second);
	}

	return true;
}