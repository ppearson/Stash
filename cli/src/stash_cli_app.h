/*
 * Stash:  A Personal Finance app (WIP CLI version).
 * Copyright (C) 2020-2021 Peter Pearson
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

#ifndef STASH_CLI_APP_H
#define STASH_CLI_APP_H

#include <string>

#include "../../core/document.h"

class StashCLIApp
{
public:
	StashCLIApp();

	bool loadFile(const std::string& filePath);

	void listAccounts(bool showBalances) const;

	void listAccountTransactions(unsigned int accountIndex, unsigned int numRecentTransactions) const;
	
	void printDatesAndBalances(unsigned int accountIndex) const;
	
	void printTotalDatesAndBalances() const;
	
	void test() const;

protected:

	Document	m_document;
};

#endif // STASH_CLI_APP_H
