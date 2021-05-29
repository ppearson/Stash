/*
 * Stash:  A Personal Finance app (WIP CLI version).
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

#include <stdio.h>

#include "stash_cli_app.h"

void printUsage()
{
	fprintf(stderr, "Stash command line interface.\n");
	fprintf(stderr, "Usage: stash_cli <filename> <command> <command args>\n");
	fprintf(stderr, "Commands:\n");
	fprintf(stderr, "  lista:\t\tList account names\n");
	fprintf(stderr, "  listab:\t\tList account names along with balance of each\n");
	fprintf(stderr, "  listt:\t\tList account Transactions\n");
	fprintf(stderr, "  print:\t\tPrint CSV values of dates and balances to stdout for specified account index\n");
	fprintf(stderr, "  printall:\t\tPrint CSV values of dates and balances to stdout for all accounts\n");
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage();
		return -1;
	}

	std::string filePath = argv[1];

	StashCLIApp app;
	if (filePath == "test")
	{
		app.test();
		return 0;
	}
	
	std::string command = argv[2];

	if (!app.loadFile(filePath))
	{
		fprintf(stderr, "Can't open file: %s\n", filePath.c_str());
		return -1;
	}

	if (command == "lista")
	{
		app.listAccounts(false);
	}
	else if (command == "listab")
	{
		app.listAccounts(true);
	}
	else if (command == "listt")
	{
		app.listAccountTransactions(0, 10);
	}
	else if (command == "print")
	{
		unsigned int accountIndex = 0;
		if (argc > 3)
		{
			accountIndex = atoi(argv[3]);
		}
		app.printDatesAndBalances(accountIndex);
	}
	else if (command == "printall")
	{
		app.printTotalDatesAndBalances();
	}

	return 0;
}
