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

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>

#include "split_transaction.h"
#include "storage.h"

SplitTransaction::SplitTransaction(const std::string& Description, const std::string& Payee, const std::string& Category, fixed Amount) :
		m_Description(Description), m_Payee(Payee), m_Category(Category), m_Amount(Amount)
{

}

void SplitTransaction::Load(std::fstream &stream, int version)
{
	Storage::LoadString(m_Description, stream);
	Storage::LoadString(m_Payee, stream);
	Storage::LoadString(m_Category, stream);
	m_Amount.Load(stream, version);	
}

void SplitTransaction::Store(std::fstream &stream) const
{
	Storage::StoreString(m_Description, stream);
	Storage::StoreString(m_Payee, stream);
	Storage::StoreString(m_Category, stream);
	m_Amount.Store(stream);
}
