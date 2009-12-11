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

class GraphValue
{
public:
	GraphValue(std::string title, double angle, fixed amount);
	~GraphValue() { };
	
	void setTitle(std::string title) { m_title = title; }
	void setAngle(double angle) { m_angle = angle; }
	void setAmount(fixed amount) { m_amount = amount; }
	
	bool operator<(const GraphValue &rhs) const
	{
		return this->m_angle < rhs.m_angle;
//		return rhs.m_angle < this->m_angle;
	}
	
	std::string		getTitle() { return m_title; }
	double			getAngle() { return m_angle; }
	fixed			getAmount() { return m_amount; }	
	
protected:
	std::string	m_title;
	double		m_angle;
	fixed		m_amount;
};


bool buildItemsForExpenseCategories(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers);
bool buildItemsForExpensePayees(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers);
bool buildItemsForDepositCategories(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers);
bool buildItemsForDepositPayees(Account *pAccount, std::vector<GraphValue> &aValues, Date &startDate, Date &endDate, fixed &overallTotal, bool ignoreTransfers);

void copyItemsToVector(std::map<std::string, fixed> &aMap, std::vector<GraphValue> &aVector, fixed &overallTotal);
