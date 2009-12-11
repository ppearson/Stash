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
#include <algorithm>
#include "analysis.h"
#include "transaction.h"

GraphValue::GraphValue(std::string title, double angle, fixed amount) : m_title(title), m_angle(angle), m_amount(amount)
{

}

bool buildItemsForExpenseCategories(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
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
	
	copyItemsToVector(aMap, aValues, overallTotal);

	return true;
}

bool buildItemsForExpensePayees(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
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
	
	copyItemsToVector(aMap, aValues, overallTotal);
	
	return true;
}

bool buildItemsForDepositCategories(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
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
	
	copyItemsToVector(aMap, aValues, overallTotal);
	
	return true;	
}

bool buildItemsForDepositPayees(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers)
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
	
	copyItemsToVector(aMap, aValues, overallTotal);
	
	return true;	
}

void copyItemsToVector(std::map<std::string, fixed> &aMap, std::vector<GraphValue> &aVector, fixed &overallTotal)
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
			GraphValue newGraphValue(title, dPieAngle, amount);
			
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
		
		GraphValue newGraphValue("Other", dPieAngle, leftovers);
		
		aVector.push_back(newGraphValue);
	}	
}
