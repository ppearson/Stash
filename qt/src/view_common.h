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

#ifndef VIEW_COMMON_H
#define VIEW_COMMON_H

enum DocumentIndexType
{
	eDocIndex_None,
	eDocIndex_Heading, //
	eDocIndex_Account,
	eDocIndex_ManagePayees,
	eDocIndex_ManageCategories,
	eDocIndex_ManageScheduledTransactions,
	eDocIndex_Graph
};

enum TransactionsViewDurationType
{
	eTransViewShowRecent,
	eTransViewShowThisYear,
	eTransViewShowAll
};

#endif // VIEW_COMMON_H

