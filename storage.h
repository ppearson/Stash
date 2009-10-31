#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <stdio.h>
#include <fstream>

#include "account.h"


bool exportAccountToQIFFile(Account *pAccount, std::string path, DateStringFormat dateFormat);
bool importQIFFileToAccount(Account *pAccount, std::string path, DateStringFormat dateFormat, char cDateFieldSep);
bool getDateFormatSampleFromQIFFile(std::string path, std::string &sample);

void clearTransaction(Transaction &trans);
void clearSplitTransaction(SplitTransaction &split);

std::string accountTypeToString(AccountType type);

#endif
