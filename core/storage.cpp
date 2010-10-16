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

#include "storage.h"

bool exportAccountToQIFFile(Account *pAccount, std::string path, DateStringFormat dateFormat)
{
	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::trunc);
	
	if (!fileStream)
	{
		return false;
	}

	// output Account header line

	fileStream << "!Account\n";
	fileStream << "N" << pAccount->getName() << "\n";
	fileStream << "^\n";
	fileStream << "!Type:" << accountTypeToString(pAccount->getType()) << "\n";

	// output each transaction record

	std::vector<Transaction>::const_iterator it = pAccount->begin();
	std::vector<Transaction>::const_iterator itEnd = pAccount->end();
	for (; it != itEnd; ++it)
	{
		Date date = it->getDate();
		fixed amount = it->getAmount();

		fileStream << "D" << date.FormattedDate(dateFormat) << "\n";
		fileStream << "P" << it->getPayee() << "\n";
		fileStream << "T" << amount << "\n";

		if (!it->getDescription().empty())
		{
			fileStream << "M" << it->getDescription() << "\n";
		}

		if (it->isReconciled())
		{
			fileStream << "C" << "R\n";
		}

		if (!it->getCategory().empty())
		{
			fileStream << "L" << it->getCategory() << "\n";
		}

		// now do splits

		int numSplits = it->getSplitCount();

		for (int i = 0; i < numSplits; i++)
		{
			SplitTransaction split = it->getSplit(i);

			fileStream << "S" << split.getCategory() << "\n";
			fileStream << "E" << split.getPayee() << "\n";
			fileStream << "$" << split.getAmount() << "\n";
		}

		fileStream << "^\n";
	}
	
	fileStream.close();

	return true;
}

bool importQIFFileToAccount(Account *pAccount, std::string path, DateStringFormat dateFormat, char cDateFieldSep, bool bMarkCleared)
{
	bool bDOSFormat = false;
	char lineEndings;
	if (!getLineEndingsFromFile(path, lineEndings, bDOSFormat))
		return false;

	char buf[1024];
	memset(buf, 0, 1024);

	std::fstream fileStream(path.c_str(), std::ios::in);
	
	// we currently only support importing data into the requested Account and don't create
	// new accounts as needed, so we don't need to bother with the account stuff yet

	std::string line;

	Transaction newTransaction;
	SplitTransaction newSplit;
	
	bool bPastHeader = false;

	while (fileStream.getline(buf, 1024, lineEndings))
	{
		line.assign(buf);

		if (line.length() == 0)
			continue;
		
		if (bDOSFormat && (line.find('\n') != std::string::npos))
			line = line.erase(0, 1);

		std::string itemData;

		char code = line[0];

		if (line.length() > 1)
		{
			// get hold of the item data
			itemData = line.substr(1);

			if (code == 'D')
			{
				Date date(itemData, cDateFieldSep, dateFormat);

				newTransaction.setDate(date);
			}
			else if (code == 'T')
			{
				fixed amount(itemData);

				newTransaction.setAmount(amount);
			}
			else if (code == 'C')
			{
				bool bCleared = false;

				if (itemData == "X" || itemData == "R")
				{
					bCleared = true;
				}

				newTransaction.setCleared(bCleared);
			}
			else if (code == 'P')
			{
				newTransaction.setPayee(itemData);
			}
			else if (code == 'L')
			{
				newTransaction.setCategory(itemData);
			}
			else if (code == 'M')
			{
				newTransaction.setDescription(itemData);
			}
			// splits - we're assuming that the $ item will be the last, so that we know when a new
			// split item starts - this might not always be that case...
			else if (code == 'S')
			{
				newSplit.setCategory(itemData);
			}
			else if (code == 'E')
			{
				newSplit.setPayee(itemData);
			}
			else if (code == '$')
			{
				fixed amount(itemData);

				newSplit.setAmount(amount);

				newTransaction.addSplit(newSplit);
				newTransaction.setSplit(true);
				clearSplitTransaction(newSplit);
			}
		}
		else
		{
			if (code == '^')
			{
				if (!bPastHeader)
				{
					bPastHeader = true;
					continue;
				}
				
				if (bMarkCleared)
					newTransaction.setCleared(true);
				
				if (bPastHeader)
					pAccount->addTransaction(newTransaction);

				clearTransaction(newTransaction);
				clearSplitTransaction(newSplit);
			}
		}

		memset(buf, 0, 1024);
	}

	fileStream.close();

	return true;
}

bool getDateFormatSampleFromQIFFile(std::string path, std::string &sample)
{
	bool bDOSFormat = false;
	char lineEndings;
	if (!getLineEndingsFromFile(path, lineEndings, bDOSFormat))
		return false;
	
	std::fstream fileStream(path.c_str(), std::ios::in);
	
	std::string line;
	
	char buf[1024];
	memset(buf, 0, 1024);
	
	while (fileStream.getline(buf, 1024, lineEndings))
	{
		line.assign(buf);
		
		if (line.length() == 0)
			continue;
		
		if (bDOSFormat && (line.find('\n') != std::string::npos))
			line = line.erase(0, 1);
		
		std::string itemData;
		
		char code = line[0];
		
		if (line.length() > 1)
		{
			// get hold of the item data
			itemData = line.substr(1);
			
			if (code == 'D')
			{
				sample = itemData;
				break;
			}
		}
	}
	
	fileStream.close();
	
	return true;	
}

std::string accountTypeToString(AccountType type)
{
	std::string QIFType;

	switch (type)
	{
		case Cash:
			QIFType = "Cash";
			break;
		case CreditCard:
			QIFType = "CCard";
			break;
		case Investment:
			QIFType = "Invst";
			break;
		case Asset:
			QIFType = "Oth A";
			break;
		case Liability:
			QIFType = "Oth L";
			break;
		case Checking:
		default:
			QIFType = "Bank";
			break;
	}

	return QIFType;
}

void clearTransaction(Transaction &trans)
{
	trans.setAmount(0.0);
	trans.setCategory("");
	trans.setDate(-1);
	trans.setDescription("");
	trans.setPayee("");
	trans.setReconciled(false);
	trans.setType(None);
	trans.setSplit(false);

	int numSplits = trans.getSplitCount();

	for (int i = 0; i < numSplits; i++)
	{
		trans.deleteSplit(i);
	}
}

void clearSplitTransaction(SplitTransaction &split)
{
	split.setAmount(0.0);
	split.setCategory("");
	split.setDescription("");
	split.setPayee("");
}

bool getLineEndingsFromFile(const std::string path, char &endChar, bool &bDOSFormat)
{
	std::fstream testStream(path.c_str(), std::ios::in);
	if (!testStream)
	{
		return false;
	}
	
	char buf[1024];
	memset(buf, 0, 1024);
	
	testStream.get(buf, 1024, -1);
	std::string strTest(buf);
	
	endChar = '\n';
	bDOSFormat = false;
	if (strTest.find("\r\n") != std::string::npos)
	{
		bDOSFormat = true;
		endChar = '\r';
	}
	else if (strTest.find('\r') != std::string::npos)
	{
		endChar = '\r';
	}
	else if (strTest.find('\n') != std::string::npos)
	{
		endChar = '\n';
	}
	
	testStream.close();
	
	return true;
}
