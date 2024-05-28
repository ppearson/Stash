/*
 * Stash:  A Personal Finance app (Qt UI).
 * Copyright (C) 2020 Peter Pearson
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

#ifndef GRAPH_COMMON_H
#define GRAPH_COMMON_H

#include <string>
#include <set>

#include "../../core/graph.h"

// struct to hold temporary graph params state before applied to actual graph
// TODO: why don't we just use a copy of Graph itself rather than duplicating
//       things?
struct TempGraphParamState
{
	int				accountIndex = -1;
	
	Graph::Type		dataType = Graph::ExpenseCategories;

	Graph::DateType	dateType = Graph::DateWeek;
	
	Date			startDate;
	Date			endDate;
	
	bool			ignoreTransfers = false;
	
	Graph::ItemsType	itemType = Graph::AllItems;
	std::set<std::string>	aItems;
};

#endif // GRAPH_COMMON_H

