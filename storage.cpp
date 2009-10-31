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

	std::vector<Transaction>::iterator it = pAccount->begin();
	for (; it != pAccount->end(); ++it)
	{
		Date date = it->Date1();
		fixed amount = it->Amount();

		fileStream << "D" << date.FormattedDate(dateFormat) << "\n";
		fileStream << "P" << it->Payee() << "\n";
		fileStream << "T" << amount << "\n";

		if (!it->Description().empty())
		{
			fileStream << "M" << it->Description() << "\n";
		}

		if (it->isReconciled())
		{
			fileStream << "C" << "R\n";
		}

		if (!it->Category().empty())
		{
			fileStream << "L" << it->Category() << "\n";
		}

		// now do splits

		int numSplits = it->getSplitCount();

		for (int i = 0; i < numSplits; i++)
		{
			SplitTransaction split = it->getSplit(i);

			fileStream << "S" << split.Category() << "\n";
			fileStream << "E" << split.Payee() << "\n";
			fileStream << "$" << split.Amount() << "\n";
		}

		fileStream << "^\n";
	}
	
	fileStream.close();

	return true;
}

bool importQIFFileToAccount(Account *pAccount, std::string path, DateStringFormat dateFormat, char cDateFieldSep)
{
	// work out what type of line endings we have

	std::fstream testStream(path.c_str(), std::ios::in);
	if (!testStream)
	{
		return false;
	}

	char buf[1024];
	memset(buf, 0, 1024);

	testStream.get(buf, 1024, -1);
	std::string strTest(buf);

	char lineEndings = '\n';
	bool bDOSFormat = false;
	if (strTest.find("\r\n") != -1)
	{
		bDOSFormat = true;
		lineEndings = '\r';
	}
	else if (strTest.find('\r') != -1)
	{
		lineEndings = '\r';
	}
	else if (strTest.find('\n') != -1)
	{
		lineEndings = '\n';
	}
	
	testStream.close();

	std::fstream fileStream(path.c_str(), std::ios::in);
	
	// we currently only support importing data into the requested Account and don't create
	// new accounts as needed, so we don't need to bother with the account stuff yet

	std::string line;

	memset(buf, 0, 1024);

	Transaction newTransaction;
	SplitTransaction newSplit;

	while (fileStream.getline(buf, 1024, lineEndings))
	{
		line.assign(buf);

		if (line.length() == 0)
			continue;
		
		if (bDOSFormat && (line.find('\n') != -1))
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
				bool bReconciled = false;

				if (itemData == "X" || itemData == "R")
				{
					bReconciled = true;
				}

				newTransaction.setReconciled(bReconciled);
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
	// work out what type of line endings we have
	
	std::fstream testStream(path.c_str(), std::ios::in);
	if (!testStream)
	{
		return false;
	}
	
	char buf[1024];
	memset(buf, 0, 1024);
	
	testStream.get(buf, 1024, -1);
	std::string strTest(buf);
	
	char lineEndings = '\n';
	bool bDOSFormat = false;
	if (strTest.find("\r\n") != -1)
	{
		bDOSFormat = true;
		lineEndings = '\r';
	}
	else if (strTest.find('\r') != -1)
	{
		lineEndings = '\r';
	}
	else if (strTest.find('\n') != -1)
	{
		lineEndings = '\n';
	}
	
	testStream.close();
	
	std::fstream fileStream(path.c_str(), std::ios::in);
	
	// we currently only support importing data into the requested Account and don't create
	// new accounts as needed, so we don't need to bother with the account stuff yet
	
	std::string line;
	
	memset(buf, 0, 1024);
	
	while (fileStream.getline(buf, 1024, lineEndings))
	{
		line.assign(buf);
		
		if (line.length() == 0)
			continue;
		
		if (bDOSFormat && (line.find('\n') != -1))
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

	int numSplits = trans.getSplitCount() - 1;

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