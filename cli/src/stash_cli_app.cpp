/*
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C)  2020 Peter Pearson
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

#include "stash_cli_app.h"

StashCLIApp::StashCLIApp()
{

}

bool StashCLIApp::loadFile(const std::string& filePath)
{
	std::fstream fileStream(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!fileStream)
	{
		return false;
	}

	// hacky, but...
	// this will be set to true if Load() returns false due to a future file format
	// version we don't know about...
	bool isFutureFileVersion = false;

	if (!m_document.Load(fileStream, isFutureFileVersion))
	{
		fileStream.close();

		fprintf(stderr, "Error: Unrecognised stash file format version.\nIt's likely that the file format of the Stash document you are trying to open is from a newer version of Stash.\n");

		return false;
	}

	fileStream.close();

	return true;
}

void StashCLIApp::listAccounts(bool showBalances) const
{
	fprintf(stderr, "Number of accounts: %u\n", m_document.getAccountCount());

	unsigned int index = 0;

	const std::vector<Account>& aAccounts = m_document.getAccounts();
	for (const Account& account : aAccounts)
	{
		if (!showBalances)
		{
			fprintf(stderr, "%u: %s\n", index++, account.getName().c_str());
		}
		else
		{
			fixed balance = account.getBalance(true, -1);
			double dBalance = balance.ToDouble();

			fprintf(stderr, "%u: %s:\t$%0.2f\n", index++, account.getName().c_str(), dBalance);
		}
	}

	if (showBalances)
	{
		fixed totalBalance = m_document.getBalance(true);
		double dTotalBalance = totalBalance.ToDouble();

		fprintf(stderr, "\nTotal balace: $%0.2f\n", dTotalBalance);
	}
}

void StashCLIApp::listAccountTransactions(unsigned int accountIndex, unsigned int numRecentTransactions) const
{
	if (accountIndex >= m_document.getAccountCount())
		return;

	const Account& account = m_document.getAccounts()[accountIndex];

	bool onlyCleared = true;

	fixed balance = 0.0;

	std::vector<Transaction>::const_iterator it = account.begin();
	std::vector<Transaction>::const_iterator itEnd = account.end();

	// Note: this won't include non-cleared...
	unsigned int numTransactions = account.getTransactionCount();

	unsigned int startPrintIndex = (numTransactions <= numRecentTransactions) ? 0 : (numTransactions - numRecentTransactions);

	unsigned int count = 0;

	if (onlyCleared)
	{
		for (; it != itEnd; ++it)
		{
			const Transaction& transaction = *it;
			if (transaction.isCleared())
			{
				balance += transaction.getAmount();

				if (count >= startPrintIndex)
				{
					fprintf(stderr, "%s\t\t%s\t\t%0.2f\t\t%0.2f\n", transaction.getDate().FormattedDate(UK).c_str(), transaction.getPayee().c_str(), transaction.getAmount().ToDouble(),
											balance.ToDouble());
				}


				count ++; // TODO: again, there could be a mis-match here...
			}
		}
	}
	else
	{
		for (; it != itEnd; ++it)
		{
			balance += (*it).getAmount();
		}
	}
}

