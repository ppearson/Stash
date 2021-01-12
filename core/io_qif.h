/* 
 * Stash:  A Personal Finance app (core).
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

#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <stdio.h>
#include <fstream>

#include "account.h"

bool exportAccountToQIFFile(Account *pAccount, std::string path, Date::DateStringFormat dateFormat);
bool importQIFFileToAccount(Account *pAccount, std::string path, Date::DateStringFormat dateFormat, char cDateFieldSep, bool bMarkCleared);
bool getDateFormatSampleFromQIFFile(const std::string path, std::string &sample);
bool getLineEndingsFromFile(const std::string path, char &endChar, bool &bDOSFormat);

void clearTransaction(Transaction &trans);
void clearSplitTransaction(SplitTransaction &split);

std::string accountTypeToString(Account::Type type);

#endif
