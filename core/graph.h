/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009-2020 Peter Pearson
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

// Note: A better name for this class would maybe have been "Chart", but it's not
//       really worth changing it now...

class Graph
{
public:
	Graph();
	
	enum Type
	{
		ExpenseCategories,
		ExpensePayees,
		DepositCategories,
		DepositPayees
	};
	
	enum DateType
	{
		DateWeek,
		DateMonth,
		DateYear,
		DateCustom
	};
	
	enum ItemsType
	{
		AllItems,
		AllItemsExceptSpecified,
		OnlySpecified
	};
	
	// This is basically the chart type that's currently being displayed
	enum ViewType
	{
		Total,
		OverTime,
		Overview
	};
	
	void setName(const std::string& name) { m_name = name; }
	void setAccount(int account) { m_account = account; }
	void setViewType(ViewType type) { m_viewType = type; }
	void setStartDate(Date startDate) { m_startDate = startDate; }
	void setEndDate(Date endDate) { m_endDate = endDate; }
	void setType(Type type) { m_type = type; }
	void setIgnoreTransfers(bool igTransfers) { m_ignoreTransfers = igTransfers; }
	void setDateType(DateType type) { m_dateType = type; }
	void setItemsType(ItemsType type) { m_itemsType = type; }
	
	const std::string&		getName() const { return m_name; }
	int					getAccount() const { return m_account; }
	ViewType		getViewType() const { return m_viewType; }
	const Date&			getStartDate() const { return m_startDate; }
	const Date&			getEndDate() const { return m_endDate; }
	Type		getType() const { return m_type; }
	bool			getIgnoreTransfers() const { return m_ignoreTransfers; }
	DateType	getDateType() const { return m_dateType; }
	ItemsType	getItemsType() const { return m_itemsType; }
	
	std::set<std::string> &getItems() { return m_items; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream) const;
	
protected:
	std::string		m_name;
	int				m_account;
	ViewType		m_viewType;
	Date			m_startDate;
	Date			m_endDate;
	Type			m_type;
	bool			m_ignoreTransfers;
	DateType		m_dateType;
	ItemsType		m_itemsType;
	std::set<std::string> m_items;
};

#endif
