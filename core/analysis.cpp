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

AreaChartItem::AreaChartItem(std::string title) : m_title(title), m_maxValue(0.0)
{

}

void AreaChartItem::addAmountToValue(fixed amount)
{	
	m_amounts.push_back(amount);
	
	if (m_maxValue < amount)
		m_maxValue = amount;
}

bool buildPieChartItemsForExpenseCategories(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
{
	if (!pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::iterator it = pAccount->begin();

	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if ((*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string category = split.getCategory();
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itFind = aMap.find(category);
				
				if (itFind == aMap.end())
				{
					aMap[category] = amount;
				}
				else
				{
					fixed &catTotal = (*itFind).second;
					
					catTotal += amount;			
				}
				
				overallTotal += amount;
			}			
		}
		else
		{
			std::string category = (*it).getCategory();
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itFind = aMap.find(category);
			
			if (itFind == aMap.end())
			{
				aMap[category] = amount;
			}
			else
			{
				fixed &catTotal = (*itFind).second;
				
				catTotal += amount;			
			}
			
			overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, aValues, overallTotal);

	return true;
}

bool buildPieChartItemsForExpensePayees(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
{
	if (!pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::iterator it = pAccount->begin();
	
	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if ((*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string payee = split.getPayee();
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itFind = aMap.find(payee);
				
				if (itFind == aMap.end())
				{
					aMap[payee] = amount;
				}
				else
				{
					fixed &payeeTotal = (*itFind).second;
					
					payeeTotal += amount;			
				}
				
				overallTotal += amount;
			}			
		}
		else
		{
			std::string payee = (*it).getPayee();
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itFind = aMap.find(payee);
			
			if (itFind == aMap.end())
			{
				aMap[payee] = amount;
			}
			else
			{
				fixed &payeeTotal = (*itFind).second;
				
				payeeTotal += amount;			
			}
			
			overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, aValues, overallTotal);
	
	return true;
}

bool buildPieChartItemsForDepositCategories(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
{
	if (!pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::iterator it = pAccount->begin();

	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if (!(*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string category = split.getCategory();
				fixed amount = split.getAmount();
				
				itFind = aMap.find(category);
				
				if (itFind == aMap.end())
				{
					aMap[category] = amount;
				}
				else
				{
					fixed &catTotal = (*itFind).second;
					
					catTotal += amount;			
				}
				
				overallTotal += amount;
			}			
		}
		else
		{
			std::string category = (*it).getCategory();
			fixed amount = (*it).getAmount();
			
			itFind = aMap.find(category);
			
			if (itFind == aMap.end())
			{
				aMap[category] = amount;
			}
			else
			{
				fixed &catTotal = (*itFind).second;
				
				catTotal += amount;			
			}
			
			overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, aValues, overallTotal);
	
	return true;	
}

bool buildPieChartItemsForDepositPayees(Account *pAccount, std::vector<PieChartItem> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
{
	if (!pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::iterator it = pAccount->begin();
	
	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if (!(*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string payee = split.getPayee();
				fixed amount = split.getAmount();
				
				itFind = aMap.find(payee);
				
				if (itFind == aMap.end())
				{
					aMap[payee] = amount;
				}
				else
				{
					fixed &payeeTotal = (*itFind).second;
					
					payeeTotal += amount;			
				}
				
				overallTotal += amount;
			}			
		}
		else
		{
			std::string payee = (*it).getPayee();
			fixed amount = (*it).getAmount();
			
			itFind = aMap.find(payee);
			
			if (itFind == aMap.end())
			{
				aMap[payee] = amount;
			}
			else
			{
				fixed &payeeTotal = (*itFind).second;
				
				payeeTotal += amount;			
			}
			
			overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, aValues, overallTotal);
	
	return true;	
}

bool buildAreaChartItemsForExpenseCategories(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate,
											 fixed &overallMax, bool ignoreTransfers)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::iterator it = pAccount->begin();
	
	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if ((*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
	
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string category = split.getCategory();
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
				
				itItemFind = aItemMap.find(category);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[category] = dateMap;
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
			std::string category = (*it).getCategory();
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
			
			itItemFind = aItemMap.find(category);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[category] = dateMap;
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

	copyAreaItemsToVector(aItemMap, aDateTotals, aItems, aDates, overallMax);
	
	return true;
}

bool buildAreaChartItemsForExpensePayees(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate,
											 fixed &overallMax, bool ignoreTransfers)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::iterator it = pAccount->begin();
	
	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if ((*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string payee = split.getPayee();
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
				
				itItemFind = aItemMap.find(payee);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[payee] = dateMap;
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
			std::string payee = (*it).getPayee();
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
			
			itItemFind = aItemMap.find(payee);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[payee] = dateMap;
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
	
	copyAreaItemsToVector(aItemMap, aDateTotals, aItems, aDates, overallMax);
	
	return true;
}

bool buildAreaChartItemsForDepositCategories(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate,
											 fixed &overallMax, bool ignoreTransfers)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::iterator it = pAccount->begin();
	
	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if (!(*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string category = split.getCategory();
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
				
				itItemFind = aItemMap.find(category);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[category] = dateMap;
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
			std::string category = (*it).getCategory();
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
			
			itItemFind = aItemMap.find(category);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[category] = dateMap;
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
	
	copyAreaItemsToVector(aItemMap, aDateTotals, aItems, aDates, overallMax);
	
	return true;
}

bool buildAreaChartItemsForDepositPayees(Account *pAccount, std::vector<AreaChartItem> &aItems, std::vector<MonthYear> &aDates, Date &startDate, Date &endDate,
										 fixed &overallMax, bool ignoreTransfers)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::iterator it = pAccount->begin();
	
	for (; it != pAccount->end(); ++it)
	{
		if ((*it).getDate() < startDate || (*it).getDate() > endDate)
			continue;
		
		if (!(*it).getAmount().IsPositive())
			continue;
		
		if (ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string payee = split.getPayee();
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
				
				itItemFind = aItemMap.find(payee);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[payee] = dateMap;
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
			std::string payee = (*it).getPayee();
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
			
			itItemFind = aItemMap.find(payee);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[payee] = dateMap;
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
	
	copyAreaItemsToVector(aItemMap, aDateTotals, aItems, aDates, overallMax);
	
	return true;
}

void copyPieItemsToVector(std::map<std::string, fixed> &aMap, std::vector<PieChartItem> &aVector, fixed &overallTotal)
{
	double dOverallTotal = overallTotal.ToDouble();
	
	fixed leftovers = 0.0;
	
	// copy values to the vector
	
	std::map<std::string, fixed>::iterator itMap = aMap.begin();
	
	for (; itMap != aMap.end(); ++itMap)
	{
		std::string title = (*itMap).first;
		
		fixed amount = (*itMap).second;
		
		if (title.empty())
		{
			leftovers += amount;
			continue;
		}
		
		double dPieAngle = (amount.ToDouble() / dOverallTotal) * 360.0;
		
		if (dPieAngle > 4.0)
		{
			PieChartItem newGraphValue(title, dPieAngle, amount);
			
			aVector.push_back(newGraphValue);
		}
		else
		{
			leftovers += amount;
		}
	}
	
	// sort the values by size
	
	std::sort(aVector.begin(), aVector.end());
	
	// add other category with any leftovers that are too small to bother showing
	
	if (!leftovers.IsZero())
	{
		double dPieAngle = (leftovers.ToDouble() / dOverallTotal) * 360.0;
		
		PieChartItem newGraphValue("Other", dPieAngle, leftovers);
		
		aVector.push_back(newGraphValue);
	}	
}

void copyAreaItemsToVector(std::map<std::string, std::map< MonthYear, fixed > > &aMap, std::map<MonthYear, fixed> &aDateTotals, std::vector<AreaChartItem> &aItems,
						   std::vector<MonthYear> &aDates, fixed &overallMax)
{
	//	for each item, for each date that exists in the aDates set, create a AreaChartItem for that date and the item
		
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItem = aMap.begin();
	
	for (; itItem != aMap.end(); ++itItem)
	{
		std::string title = (*itItem).first;
		
		std::map<MonthYear, fixed>::iterator itDate = aDateTotals.begin();
		
		AreaChartItem newItem(title);
		
		for (; itDate != aDateTotals.end(); ++itDate)
		{
			std::map<MonthYear, fixed> &dateMap = (*itItem).second;
			
			MonthYear myDate = (*itDate).first;
			
			fixed &dateTotal = (*itDate).second;
			
			if (overallMax < dateTotal)
				overallMax = dateTotal;
			
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
		
		aItems.push_back(newItem);
	}
	
	// now work out what's 2% of the max overall value and if the item's less than that, remove it
	
	double dPurgeValue = overallMax.ToDouble();
	dPurgeValue *= 0.02;
	
	std::vector<AreaChartItem>::iterator itPurgeItem = aItems.begin();
	
	while (itPurgeItem != aItems.end())
	{
		if ((*itPurgeItem).getMaxValue().ToDouble() < dPurgeValue)
		{
			itPurgeItem = aItems.erase(itPurgeItem);
		}
		else
		{
			++itPurgeItem;
		}
	}
	
	// add dateitems to vector
	
	std::map<MonthYear, fixed>::iterator itDate2 = aDateTotals.begin();
	
	for (; itDate2 != aDateTotals.end(); ++itDate2)
	{
		MonthYear myDate = (*itDate2).first;
		
		aDates.push_back(myDate);		
	}
}