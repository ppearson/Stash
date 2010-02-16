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

#ifndef GRAPH_H
#define GRAPH_H

#include <set>

#include "datetime.h"

enum GraphType
{
	ExpenseCategories,
	ExpensePayees,
	DepositCategories,
	DepositPayees
};

enum GraphDateType
{
	DateWeek,
	DateMonth,
	DateYear,
	DateCustom
};

enum GraphItemsType
{
	AllItems,
	AllItemsExceptSpecified,
	OnlySpecified
};

enum GraphViewType
{
	Total,
	OverTime
};

class Graph
{
public:
	Graph();
	
	void setName(std::string name) { m_name = name; }
	void setAccount(int account) { m_account = account; }
	void setViewType(GraphViewType type) { m_viewType = type; }
	void setStartDate(Date startDate) { m_startDate = startDate; }
	void setEndDate(Date endDate) { m_endDate = endDate; }
	void setType(GraphType type) { m_type = type; }
	void setIgnoreTransfers(bool igTransfers) { m_ignoreTransfers = igTransfers; }
	void setDateType(GraphDateType type) { m_dateType = type; }
	void setItemsType(GraphItemsType type) { m_itemsType = type; }
	
	std::string		getName() { return m_name; }
	int				getAccount() { return m_account; }
	GraphViewType	getViewType() { return m_viewType; }
	Date			getStartDate() { return m_startDate; }
	Date			getEndDate() { return m_endDate; }
	const Date	   &getStartDate1() { return m_startDate; }
	const Date     &getEndDate1() { return m_endDate; }
	GraphType		getType() { return m_type; }
	bool			getIgnoreTransfers() { return m_ignoreTransfers; }
	GraphDateType	getDateType() { return m_dateType; }
	GraphItemsType	getItemsType() { return m_itemsType; }
	
	std::set<std::string> &getItems() { return m_items; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);
	
protected:
	std::string		m_name;
	int				m_account;
	GraphViewType	m_viewType;
	Date			m_startDate;
	Date			m_endDate;
	GraphType		m_type;
	bool			m_ignoreTransfers;
	GraphDateType	m_dateType;
	GraphItemsType	m_itemsType;
	std::set<std::string> m_items;
};

#endif
