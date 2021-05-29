/*
 * Stash:  A Personal Finance app (WIP CLI version).
 * Copyright (C)  2020-2021 Peter Pearson
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

#include "../../core/currency_value_formatter.h"

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

		fprintf(stderr, "\nTotal balance: $%0.2f\n", dTotalBalance);
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
					fprintf(stderr, "%s\t\t%s\t\t%0.2f\t\t%0.2f\n", transaction.getDate().FormattedDate(Date::UK).c_str(), transaction.getPayee().c_str(), transaction.getAmount().ToDouble(),
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

void StashCLIApp::printDatesAndBalances(unsigned int accountIndex) const
{
	if (accountIndex >= m_document.getAccountCount())
		return;

	const Account& account = m_document.getAccounts()[accountIndex];

	fixed balance = 0.0;

	std::vector<Transaction>::const_iterator it = account.begin();
	std::vector<Transaction>::const_iterator itEnd = account.end();

	// Note: this won't include non-cleared...
	unsigned int numTransactions = account.getTransactionCount();

//	unsigned int startPrintIndex = (numTransactions <= numRecentTransactions) ? 0 : (numTransactions - numRecentTransactions);

	unsigned int count = 0;

	for (; it != itEnd; ++it)
	{
		balance += (*it).getAmount();
		
		const Transaction& transaction = *it;
		
		fprintf(stdout, "%s,%0.2f\n", transaction.getDate().FormattedDate(Date::UK).c_str(), balance.ToDouble());
	}
}

void StashCLIApp::printTotalDatesAndBalances() const
{
	// find starting date
	unsigned int numAccounts = m_document.getAccountCount();
	Date firstDate;
	firstDate.Now();
	for (unsigned int i = 0; i < numAccounts; i++)
	{
		const Account& account = m_document.getAccounts()[i];
		if (account.getTransactionCount() == 0)
			continue;
		
		Date firstTransactionDate = account.getTransaction(0).getDate();
		if (firstTransactionDate < firstDate)
		{
			firstDate = firstTransactionDate;
		}
	}
	
	Date endDate;
	endDate.Now();
	
	Date testDate = firstDate;
	
	fixed currentBalance = 0.0;
	
	while (testDate < endDate)
	{
		// brute-force find any transactions for the date, and accumulate them...
		
		// this is hilariously bad complexity-wise, but given it's still pretty much instant in terms of speed with < 10,000 transactions over 6 accounts,
		// easiest for the moment...
		// TODO: keep a note of per-account indices we've checked already to restart there next time, or something better...
		
		bool foundSomething = false;
		
		for (unsigned int i = 0; i < numAccounts; i++)
		{
			const Account& account = m_document.getAccounts()[i];
			if (account.getTransactionCount() == 0)
				continue;
			
			std::vector<Transaction>::const_iterator it = account.begin();
			std::vector<Transaction>::const_iterator itEnd = account.end();
			
			// start with the date of the first transaction
			Date previousDate = account.getTransaction(0).getDate();
			
			for (; it != itEnd; ++it)
			{
				const Transaction& transaction = *it;
				
				// detect if any dates might be in the wrong order...
				if (transaction.getDate() < previousDate)
				{
					// print a warning to stderr (very verbose due to the brute-force algorithm, but...)
					fprintf(stderr, "Warning: unexpected date order: account index: %u, this date: %s, previous date: %s\n",
							i, transaction.getDate().FormattedDate(Date::UK).c_str(), previousDate.FormattedDate(Date::UK).c_str());
				}
				previousDate = transaction.getDate();
				
				if (transaction.getDate() == testDate)
				{
					currentBalance += (*it).getAmount();
					foundSomething = true;
				}
				else if (transaction.getDate() > testDate)
				{
					// won't find it...
					break;
				}
			}
		}
		
		if (foundSomething)
		{
			fprintf(stdout, "%s,%0.2f\n", testDate.FormattedDate(Date::UK).c_str(), currentBalance.ToDouble());
		}
		
		testDate.IncrementDays(1);
	}
}

void StashCLIApp::test() const
{
	fixed v1 = 4332.42;
	fixed v2 = 415.33;
	fixed v3 = 10.24;
	
	CurrencyValueFormatter formatter;
	
	fprintf(stderr, "val: %0.2f\t\tlen: %u\n", v1.ToDouble(), v1.GetNumDigits());
	fprintf(stderr, "val: %0.2f\t\tlen: %u\n", v2.ToDouble(), v2.GetNumDigits());
	fprintf(stderr, "val: %0.2f\t\tlen: %u\n", v3.ToDouble(), v3.GetNumDigits());
	
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(v1));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(v2));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(v3));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(31.30));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(311.30));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(3167.30));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(31678.30));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(-3167452.30));
	
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(0.0));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(0.50));
	fprintf(stderr, "Formatted: %s\n", formatter.formatValue(0.01));
}

