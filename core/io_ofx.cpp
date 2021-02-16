/* 
 * Stash:  A Personal Finance app (core).
 * Copyright (C) 2010-2021 Peter Pearson
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

#include <locale.h>
#include <iconv.h>

#include "io_ofx.h"
#include "io_qif.h"
#include "account.h"
#include "string.h"

OFXBankAccount::OFXBankAccount(const std::string& bankID, const std::string& accountID, const std::string& type) :
						m_bankID(bankID), m_accountID(accountID)
{
	m_type = Account::eTypeChecking;
}

OFXStatementResponse::OFXStatementResponse(const OFXBankAccount &account) : m_account(account)
{
	
}

fixed OFXStatementResponse::calculateBalance() const
{
	fixed balance = 0.0;
	
	OFXStTrIt transactionsIt = begin();
	
	for (; transactionsIt != end(); ++transactionsIt)
	{
		const OFXStatementTransaction& itemTrans = (*transactionsIt);
		
		balance += itemTrans.getAmount();			
	}
	
	return balance;
}

void OFXStatementResponse::writeToSGMLStream(std::fstream &stream) const
{
	stream << "      <STMTRS>\n";
	stream << "      <CURDEF>" << m_currency << "\n";
	stream << "      <BANKACCTFROM>\n";
	stream << "      <BANKID>000\n";
	stream << "      <ACCTID>" << m_account.getAccountID() << "\n";
	stream << "      <ACCTTYPE>SAVINGS\n";
	stream << "      </BANKACCTFROM>\n";
	stream << "      <BANKTRANLIST>\n";
	
	Date dtStart;
	Date dtEnd;
	
	dtStart.Now();
	dtEnd.Now();
	
	int numTransactions = m_aTransactions.size();
	
	if (numTransactions > 0)
	{
		dtStart = getTransaction(0).getDate();
		dtEnd = getTransaction(numTransactions - 1).getDate();
	}
	
	stream << "      <DTSTART>" << dtStart.FormattedDate(Date::OFX) << "\n";
	stream << "      <DTEND>" << dtEnd.FormattedDate(Date::OFX) << "\n\n";
	
	std::vector<OFXStatementTransaction>::const_iterator it = m_aTransactions.begin();
		 
	for (; it != m_aTransactions.end(); ++it)
	{
		const OFXStatementTransaction& trs = (*it);
		
		trs.writeToSGMLStream(stream);		
	}
	
	stream << "      </BANKTRANLIST>\n";
	
	fixed balance = calculateBalance();
	
	stream << "      <LEDGERBAL>\n";
	stream << "       <BALAMT>" << balance << "\n";
	
	Date dt;
	dt.Now();
	
	stream << "       <DTASOF>" << dt.FormattedDate(Date::OFX) << "\n";
	stream << "      </LEDGERBAL>\n";
	stream << "     </STMTRS>\n";
}

void OFXStatementResponse::writeToXMLStream(std::fstream &stream) const
{
	stream << "      <STMTRS>\n";
	stream << "      <CURDEF>" << m_currency << "</CURDEF>\n";
	stream << "      <BANKACCTFROM>\n";
	stream << "      <BANKID>000</BANKID>\n";
	stream << "      <ACCTID>" << m_account.getAccountID() << "</ACCTID>\n";
	stream << "      <ACCTTYPE>SAVINGS</ACCTTYPE>\n";
	stream << "      </BANKACCTFROM>\n";
	stream << "      <BANKTRANLIST>\n";
	
	Date dtStart;
	Date dtEnd;
	
	dtStart.Now();
	dtEnd.Now();
	
	int numTransactions = m_aTransactions.size();
	
	if (numTransactions > 0)
	{
		dtStart = getTransaction(0).getDate();
		dtEnd = getTransaction(numTransactions - 1).getDate();
	}
	
	stream << "      <DTSTART>" << dtStart.FormattedDate(Date::OFX) << "</DTSTART>\n";
	stream << "      <DTEND>" << dtEnd.FormattedDate(Date::OFX) << "</DTEND>\n\n";
	
	std::vector<OFXStatementTransaction>::const_iterator it = m_aTransactions.begin();
	std::vector<OFXStatementTransaction>::const_iterator itEnd = m_aTransactions.end();
	
	for (; it != itEnd; ++it)
	{
		const OFXStatementTransaction& trs = (*it);
		
		trs.writeToXMLStream(stream);		
	}
	
	stream << "      </BANKTRANLIST>\n";
	
	fixed balance = calculateBalance();
	
	stream << "      <LEDGERBAL>\n";
	stream << "       <BALAMT>" << balance << "</BALAMT>\n";
	
	Date dt;
	dt.Now();
	
	stream << "       <DTASOF>" << dt.FormattedDate(Date::OFX) << "</DTASOF>\n";
	stream << "      </LEDGERBAL>\n";
	stream << "     </STMTRS>\n";
}

void OFXStatementResponse::addOFXTransactionsForAccount(const Account& account)
{
	int FITID = 1;
	std::vector<Transaction>::const_iterator it = account.begin();
	std::vector<Transaction>::const_iterator itEnd = account.end();
	for (; it != itEnd; ++it)
	{
		const Transaction &trans = (*it);
		
		std::string strName = trans.getPayee();
		std::string strMemo = trans.getDescription();
		
		OFXStatementTransaction newStatementTransaction(trans.getType(), trans.getDate(), trans.getAmount(), strName, FITID++);
		newStatementTransaction.setMemo(strMemo);
		
		m_aTransactions.push_back(newStatementTransaction);		
	}
	
	m_account.setType(account.getType());
	m_account.setAccountID(account.getName());
}

OFXStatementTransaction::OFXStatementTransaction(const Transaction::Type &type, const Date &date, const fixed &amount, std::string &name, int FITID) :
									m_type(type), m_date(date), m_amount(amount), m_name(name)
{
	std::stringstream ss;
	ss << FITID;
	
	m_fitid = ss.str();
}

void OFXStatementTransaction::setDate(const std::string &date)
{
	m_date.setDate(date, 0, Date::OFX);
}

void OFXStatementTransaction::writeToSGMLStream(std::fstream &stream) const
{
	stream << "        <STMTTRN>\n";
	stream << "          <TRNTYPE>" << TransactionTypeToString(m_type) << "\n";
	stream << "          <DTPOSTED>" << m_date.FormattedDate(Date::OFX) << "\n";
	stream << "          <TRNAMT>" << m_amount << "\n";
	stream << "          <FITID>" << m_fitid << "\n";
	stream << "          <NAME>" << m_name << "\n";
	
	if (!m_memo.empty())
	{
		stream << "          <MEMO>" << m_memo << "\n";
	}
	
	stream << "        </STMTTRN>\n\n";
}

void OFXStatementTransaction::writeToXMLStream(std::fstream &stream) const
{
	stream << "        <STMTTRN>\n";
	stream << "          <TRNTYPE>" << TransactionTypeToString(m_type) << "</TRNTYPE>\n";
	stream << "          <DTPOSTED>" << m_date.FormattedDate(Date::OFX) << "</DTPOSTED>\n";
	stream << "          <TRNAMT>" << m_amount << "</TRNAMT>\n";
	stream << "          <FITID>" << m_fitid << "</FITID>\n";
	stream << "          <NAME>" << m_name << "</NAME>\n";
	
	if (!m_memo.empty())
	{
		stream << "          <MEMO>" << m_memo << "</MEMO>\n";
	}
	
	stream << "        </STMTTRN>\n\n";
}

void OFXBankAccount::setType(const std::string& type)
{
	m_type = StringToAccountType(type);
}

void OFXStatementTransaction::setType(const std::string& type)
{
	m_type = StringToTransactionType(type);
}

void OFXStatementTransactionResponse::writeToSGMLStream(std::fstream &stream) const
{
	stream << "   <STMTTRNRS>\n";
	stream << "     <TRNUID>1\n";
	stream << "     <STATUS>\n";
	stream << "      <CODE>0\n";
	stream << "      <SEVERITY>INFO\n";
	stream << "    </STATUS>\n";
	
	m_statementResponse.writeToSGMLStream(stream);
	
	stream << "   </STMTTRNRS>\n";
}

void OFXStatementTransactionResponse::writeToXMLStream(std::fstream &stream) const
{
	stream << "   <STMTTRNRS>\n";
	stream << "     <TRNUID>1</TRNUID>\n";
	stream << "     <STATUS>\n";
	stream << "      <CODE>0</CODE>\n";
	stream << "      <SEVERITY>INFO</SEVERITY>\n";
	stream << "    </STATUS>\n";
	
	m_statementResponse.writeToXMLStream(stream);
	
	stream << "   </STMTTRNRS>\n";
}

bool OFXData::exportDataToFile(const std::string& path, bool xml) const
{
	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::trunc);
	
	if (!fileStream)
	{
		return false;
	}
	
	if (!xml)
		writeSGMLStartToStream(fileStream);
	else
		writeXMLStartToStream(fileStream);
	
	OFXStTrResIt responseIt = begin();
	OFXStTrResIt responseItEnd = end();
	
	for (; responseIt != responseItEnd; ++responseIt)
	{
		const OFXStatementTransactionResponse &response = (*responseIt);
		
		if (!xml)
		{
			response.writeToSGMLStream(fileStream);
		}
		else
		{
			response.writeToXMLStream(fileStream);
		}		
	}
	
	writeEndToStream(fileStream);
	
	fileStream.close();
	
	return true;
}

void OFXData::writeSGMLStartToStream(std::fstream &stream)
{
	stream << "OFXHEADER:100\n";
	stream << "DATA:OFXSGML\n";
	stream << "VERSION:102\n";
	stream << "SECURITY:NONE\n";
	stream << "ENCODING:UTF-8\n";
	stream << "CHARSET:NONE\n";
	stream << "COMPRESSION:NONE\n";
	stream << "OLDFILEUID:NONE\n";
	stream << "NEWFILEUID:NONE\n\n";
	
	stream << "<OFX>\n";
	stream << " <SIGNONMSGSRSV1>\n";
	stream << "   <SONRS>\n";
	stream << "    <STATUS>\n";
	stream << "     <CODE>0\n";
	stream << "     <SEVERITY>INFO\n";
	stream << "    </STATUS>\n";
	
	Date dtNow;
	dtNow.Now();
	
	stream << "    <DTSERVER>" << dtNow.FormattedDate(Date::OFX) << "\n";
	stream << "    <LANGUAGE>ENG\n";
	stream << "   </SONRS>\n";
	stream << " </SIGNONMSGSRSV1>\n\n";
	
	stream << " <BANKMSGSRSV1>\n\n";
}

void OFXData::writeXMLStartToStream(std::fstream &stream)
{
	stream << "<?xml version=\"1.0\"?>\n";
	stream << "<?OFX OFXHEADER=\"200\" VERSION=\"200\" SECURITY=\"NONE\" OLDFILEUID=\"NONE\" NEWFILEUID=\"NONE\"?>\n";
	stream << "<OFX>\n";
	stream << " <SIGNONMSGSRSV1>\n";
	stream << " <SONRS>\n";
	stream << " <STATUS>\n";
	stream << " <CODE>0</CODE>\n";
	stream << " <SEVERITY>INFO</SEVERITY>\n";
	stream << " </STATUS>\n";
	
	Date dtNow;
	dtNow.Now();
	
	stream << " <DTSERVER>" << dtNow.FormattedDate(Date::OFX) << "</DTSERVER>\n";
	stream << " <LANGUAGE>ENG</LANGUAGE>\n";
	stream << " <FI>\n";
	stream << " </FI>\n";
	stream << " </SONRS>\n";
	stream << " </SIGNONMSGSRSV1>\n";
	stream << " <BANKMSGSRSV1>\n";
}

void OFXData::writeEndToStream(std::fstream &stream)
{
	stream << " </BANKMSGSRSV1>\n";
	stream << "</OFX>\n";
}

bool detectOFXType(std::string path, OFX_TYPE &eType, std::string &encoding, std::string &charset)
{
	bool bDOSFormat = false;
	char lineEndings;
	if (!getLineEndingsFromFile(path, lineEndings, bDOSFormat))
		return false;
	
	eType = OFX_UNKNOWN;
	
	char buf[2048];
	memset(buf, 0, 2048);
	
	std::fstream fileStream(path.c_str(), std::ios::in);
		
	std::string line;
	
	int lineCount = 0;
	
	bool bXML = false;
	bool bOFX2 = false;
	bool bASCII = false;
	
	charset = "UTF-8";
	
	// go over the first 5 lines in the file and try and work out whether it's OFX 1.x or 2.x
	while (fileStream.getline(buf, 2048, lineEndings) && lineCount++ < 12)
	{
		line.assign(buf);
		
		if (line.length() == 0)
			continue;
		
		if (bDOSFormat && (line.find('\n') != std::string::npos))
			line = line.erase(0, 1);
		
		if (line.find("DATA:OFXSGML") != std::string::npos)
		{
			
		}
		else if (line.find("<?") != std::string::npos)
		{
			bXML = true;
		}
		else if (line.find("OFXHEADER=\"2") != std::string::npos)
		{
			bOFX2 = true;
		}
		else if (line.find("ENCODING:USASCII") != std::string::npos)
		{
			bASCII = true;
		}
		else if (line.find("CHARSET:1252") != std::string::npos)
		{
			charset = "1252";
		}
		else if (line.find("CHARSET:ISO-8859-1") != std::string::npos)
		{
			charset = "ISO-8859-1";
		}
	}
	
	fileStream.close();
	
	if (bXML && bOFX2)
		eType = OFX_XML;
	else
		eType = OFX_SGML;
	
	if (bASCII)
	{
		encoding = "USASCII";
	}
	
	return true;
}

bool importOFXFile(std::string path, OFXData &dataItem)
{
	OFX_TYPE eType = OFX_UNKNOWN;
	
	std::string encoding;
	std::string charset;
	
	if (!detectOFXType(path, eType, encoding, charset))
		return false;
	
	// this *should* be able to cope with importing 2.0 XML files as well
	return importOFXSGMLFile(path, dataItem, encoding, charset);
}

bool importOFXSGMLFile(std::string path, OFXData &dataItem, std::string &encoding, std::string &charset)
{
	bool bDOSFormat = false;
	char lineEndings;
	if (!getLineEndingsFromFile(path, lineEndings, bDOSFormat))
		return false;
	
	iconv_t conversion_descriptor;
	
	std::string fromCharset;
	
	if (encoding == "USASCII")
	{
		if (charset == "1252")
		{
			fromCharset = "CP1252";
		}
		else if (charset == "ISO-8859-1")
		{
			fromCharset = "ISO-8859-1";
		}
		else
		{
			fromCharset = "USASCII";
		}		
	}
	else
	{
		fromCharset = "UTF-8";
	}
	
	conversion_descriptor = iconv_open("UTF-8//IGNORE", fromCharset.c_str());
	
	char buf[1024];
	memset(buf, 0, 1024);
	
	char decodedBuf[1024*2];
		
	std::fstream fileStream(path.c_str(), std::ios::in);
	
	std::string line;
	
	OFXStatementTransactionResponse statementTransactionResponse;
	OFXStatementResponse statementResponse;
	OFXBankAccount bankAccount;
	OFXStatementTransaction transaction;
	
	size_t decodedLength = 2048;
	
	while (fileStream.getline(buf, 1024, lineEndings))
	{
		std::string encoded(buf);
		memset(decodedBuf, 0, 1024*2);
		char *decodedBufferChar = decodedBuf;
		
		char *encodedChar = (char*)encoded.c_str();		
		size_t encodedLength = strlen(encodedChar);
		
		decodedLength = 2048;
		
		int iconv_retval = iconv(conversion_descriptor, &encodedChar, &encodedLength, &decodedBufferChar, &decodedLength);
		
		if (iconv_retval != -1)
		{
			line.assign(decodedBuf);
		}
		else
		{
			line.assign(buf);
		}
		
		if (line.length() == 0)
			continue;
		
		if (bDOSFormat && (line.find('\n') != std::string::npos))
			line = line.erase(0, 1);
		
		size_t nTagStart = line.find('<');
		size_t nTagEnd = line.find('>');
		
		if (nTagStart != std::string::npos && nTagEnd > nTagStart)
		{
			std::string tag = line.substr(nTagStart + 1, nTagEnd - nTagStart - 1);
			
			std::string data;
			
			if (nTagEnd < line.size())
			{
				data = line.substr(nTagEnd + 1);
			}
			
			// handle possible OFX 1.x formats with closing tags
			int nEndTag = data.find('<');
			if (nEndTag >= 0)
			{
				data = data.substr(0, nEndTag);
			}
			
			if (tag == "STMTTRNRS" || tag == "CCSTMTTRNRS")
			{
				statementTransactionResponse = OFXStatementTransactionResponse();
			}
			else if (tag == "/STMTTRNRS" || tag == "/CCSTMTTRNRS")
			{
				dataItem.addStatementTransactionResponse(statementTransactionResponse);
			}
			else if (tag == "STMTRS" || tag == "CCSTMTRS")
			{
				statementResponse = OFXStatementResponse();
			}
			else if (tag == "/STMTRS" || tag  == "/CCSTMTRS")
			{
				statementTransactionResponse.setStatementResponse(statementResponse);
			}
			else if (tag == "BANKACCTFROM")
			{
				bankAccount = OFXBankAccount();
			}
			else if (tag == "CCACCTFROM")
			{
				bankAccount = OFXBankAccount();
				
				bankAccount.setType(Account::eTypeCreditCard);
			}
			else if (tag == "BANKID")
			{
				bankAccount.setBankID(data);
			}
			else if (tag == "ACCTID")
			{
				bankAccount.setAccountID(data);
			}
			else if (tag == "ACCTTYPE")
			{
				bankAccount.setType(data);
			}
			else if (tag == "/BANKACCTFROM" || tag == "/CCACCTFROM")
			{
				statementResponse.setAccount(bankAccount);
			}
			else if (tag == "STMTTRN")
			{
				transaction = OFXStatementTransaction();
			}
			else if (tag == "TRNTYPE")
			{
				transaction.setType(data);
			}
			else if (tag == "DTPOSTED")
			{
				transaction.setDate(data);
			}
			else if (tag == "TRNAMT")
			{
				transaction.setAmount(data);
			}
			else if (tag == "NAME")
			{
				transaction.setName(data);
			}
			else if (tag == "MEMO")
			{
				transaction.setMemo(data);
			}
			else if (tag == "FITID")
			{
				transaction.setFITID(data);
			}
			else if (tag == "/STMTTRN")
			{
				statementResponse.addTransaction(transaction);
			}
			else if (tag == "BALAMT")
			{
				statementResponse.setBalance(data);
			}
		}		
	}
	
	fileStream.close();

	return true;
}

bool importOFXStatementIntoAccount(Account &account, const OFXStatementResponse &statement, bool reverseTransactions, bool cleared, bool ignoreDuplicates)
{
	if (reverseTransactions)
	{
		// insanely disgusting, and this whole infrastructure needs a re-factor, but optionally using reverse_iterators
		// below is not completely trivial due to differing types, so this is easiest for the moment...
		OFXStatementResponse& nonConstStatement = const_cast<OFXStatementResponse&>(statement);
		nonConstStatement.reverseTransactionOrder();
	}
	
	OFXStTrIt transactionsIt = statement.begin();
	OFXStTrIt transactionsItEnd = statement.end();
	
	bool ignoreThisOne = false;
	
	for (; transactionsIt != transactionsItEnd; ++transactionsIt)
	{
		const OFXStatementTransaction& itemTrans = *transactionsIt;
		
		Transaction newTransaction(itemTrans.getMemo(), itemTrans.getName(), "", itemTrans.getAmount(), itemTrans.getDate());
		newTransaction.setType(itemTrans.getType());
		
		if (cleared)
		{
			newTransaction.setCleared(true);
		}
		
		ignoreThisOne = false;
		
		if (!itemTrans.getFITID().empty()) // if there's a FITID, apply it to the Transaction so it will get saved
		{
			const std::string& strFITID = itemTrans.getFITID();
			newTransaction.setFITID(strFITID);
			newTransaction.setHasFITID(true);
			
			if (ignoreDuplicates)
			{
				if (account.doesFITIDExist(strFITID))
				{
					ignoreThisOne = true;
				}
			}
		}
		
		if (!ignoreThisOne)
		{
			account.addTransaction(newTransaction, true); // add the FITID as well
		}
	}
	
	return true;
}
