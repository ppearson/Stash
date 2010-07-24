/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2010 Peter Pearson
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

#include "ofx.h"
#include "storage.h"
#include "account.h"
#include "string.h"

OFXBankAccount::OFXBankAccount(std::string bankID, std::string accountID, std::string type) :
						m_bankID(bankID), m_accountID(accountID)
{
	m_type = Checking;
}

OFXStatementResponse::OFXStatementResponse(const OFXBankAccount &account) : m_account(account)
{
	
}

void OFXStatementResponse::calculateBalance()
{
	fixed balance = 0.0;
	
	OFXStTrIt transactionsIt = begin();
	
	for (; transactionsIt != end(); ++transactionsIt)
	{
		OFXStatementTransaction itemTrans = (*transactionsIt);
		
		balance += itemTrans.getAmount();			
	}
	
	m_balance = balance;
}

void OFXStatementResponse::writeToSGMLStream(std::fstream &stream)
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
	
	stream << "      <DTSTART>" << dtStart.FormattedDate(OFX) << "\n";
	stream << "      <DTEND>" << dtEnd.FormattedDate(OFX) << "\n\n";
	
	std::vector<OFXStatementTransaction>::iterator it = m_aTransactions.begin();
		 
	for (; it != m_aTransactions.end(); ++it)
	{
		OFXStatementTransaction &trs = (*it);
		
		trs.writeToSGMLStream(stream);		
	}
	
	stream << "      </BANKTRANLIST>\n";
	
	calculateBalance();
	
	stream << "      <LEDGERBAL>\n";
	stream << "       <BALAMT>" << m_balance << "\n";
	
	Date dt;
	dt.Now();
	
	stream << "       <DTASOF>" << dt.FormattedDate(OFX) << "\n";
	stream << "      </LEDGERBAL>\n";
	stream << "     </STMTRS>\n";
}

void OFXStatementResponse::writeToXMLStream(std::fstream &stream)
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
	
	stream << "      <DTSTART>" << dtStart.FormattedDate(OFX) << "</DTSTART>\n";
	stream << "      <DTEND>" << dtEnd.FormattedDate(OFX) << "</DTEND>\n\n";
	
	std::vector<OFXStatementTransaction>::iterator it = m_aTransactions.begin();
	std::vector<OFXStatementTransaction>::iterator itEnd = m_aTransactions.end();
	
	for (; it != itEnd; ++it)
	{
		OFXStatementTransaction &trs = (*it);
		
		trs.writeToXMLStream(stream);		
	}
	
	stream << "      </BANKTRANLIST>\n";
	
	calculateBalance();
	
	stream << "      <LEDGERBAL>\n";
	stream << "       <BALAMT>" << m_balance << "</BALAMT>\n";
	
	Date dt;
	dt.Now();
	
	stream << "       <DTASOF>" << dt.FormattedDate(OFX) << "</DTASOF>\n";
	stream << "      </LEDGERBAL>\n";
	stream << "     </STMTRS>\n";
}

void OFXStatementResponse::addOFXTransactionsForAccount(Account &account)
{
	int FITID = 1;
	std::vector<Transaction>::iterator it = account.begin();
	std::vector<Transaction>::iterator itEnd = account.end();
	for (; it != itEnd; ++it)
	{
		Transaction &trans = (*it);
		
		std::string strName = trans.getPayee();
		std::string strMemo = trans.getDescription();
		
		OFXStatementTransaction newStatementTransaction(trans.getType1(), trans.getDate1(), trans.getAmount1(), strName, FITID++);
		newStatementTransaction.setMemo(strMemo);
		
		m_aTransactions.push_back(newStatementTransaction);		
	}
	
	m_account.setType(account.getType());
	m_account.setAccountID(account.getName());
}

OFXStatementTransaction::OFXStatementTransaction(const TransactionType &type, const Date &date, const fixed &amount, std::string &name, int FITID) :
									m_type(type), m_date(date), m_amount(amount), m_name(name)
{
	std::stringstream ss;
	ss << FITID;
	
	m_fitid = ss.str();
}

void OFXStatementTransaction::setDate(std::string &date)
{
	m_date.setDate(date, 0, OFX);
}

void OFXStatementTransaction::writeToSGMLStream(std::fstream &stream)
{
	stream << "        <STMTTRN>\n";
	stream << "          <TRNTYPE>" << TransactionTypeToString(m_type) << "\n";
	stream << "          <DTPOSTED>" << m_date.FormattedDate(OFX) << "\n";
	stream << "          <TRNAMT>" << m_amount << "\n";
	stream << "          <FITID>" << m_fitid << "\n";
	stream << "          <NAME>" << m_name << "\n";
	
	if (!m_memo.empty())
	{
		stream << "          <MEMO>" << m_memo << "\n";
	}
	
	stream << "        </STMTTRN>\n\n";
}

void OFXStatementTransaction::writeToXMLStream(std::fstream &stream)
{
	stream << "        <STMTTRN>\n";
	stream << "          <TRNTYPE>" << TransactionTypeToString(m_type) << "</TRNTYPE>\n";
	stream << "          <DTPOSTED>" << m_date.FormattedDate(OFX) << "</DTPOSTED>\n";
	stream << "          <TRNAMT>" << m_amount << "</TRNAMT>\n";
	stream << "          <FITID>" << m_fitid << "</FITID>\n";
	stream << "          <NAME>" << m_name << "</NAME>\n";
	
	if (!m_memo.empty())
	{
		stream << "          <MEMO>" << m_memo << "</MEMO>\n";
	}
	
	stream << "        </STMTTRN>\n\n";
}

void OFXBankAccount::setType(std::string type)
{
	m_type = StringToAccountType(type);
}

void OFXStatementTransaction::setType(std::string &type)
{
	m_type = StringToTransactionType(type);
}

void OFXStatementTransactionResponse::writeToSGMLStream(std::fstream &stream)
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

void OFXStatementTransactionResponse::writeToXMLStream(std::fstream &stream)
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

bool OFXData::exportDataToFile(std::string path, bool xml)
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
		OFXStatementTransactionResponse &response = (*responseIt);
		
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
	
	stream << "    <DTSERVER>" << dtNow.FormattedDate(OFX) << "\n";
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
	
	stream << " <DTSERVER>" << dtNow.FormattedDate(OFX) << "</DTSERVER>\n";
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
	
	bool bOFX1 = false;
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
		
		if (bDOSFormat && (line.find('\n') != -1))
			line = line.erase(0, 1);
		
		if (line.find("DATA:OFXSGML") != -1)
		{
			bOFX1 = true;
		}
		else if (line.find("<?") != -1)
		{
			bXML = true;
		}
		else if (line.find("OFXHEADER=\"2") != -1)
		{
			bOFX2 = true;
		}
		else if (line.find("ENCODING:USASCII") != -1)
		{
			bASCII = true;
		}
		else if (line.find("CHARSET:1252") != -1)
		{
			charset = "1252";
		}
		else if (line.find("CHARSET:ISO-8859-1") != -1)
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
	
	std::auto_ptr<OFXStatementTransactionResponse> pStatementTransactionResponse(new OFXStatementTransactionResponse());
	std::auto_ptr<OFXStatementResponse> pStatementResponse(new OFXStatementResponse());
	std::auto_ptr<OFXBankAccount> pBankAccount(new OFXBankAccount());
	std::auto_ptr<OFXStatementTransaction> pTransaction(new OFXStatementTransaction());
	
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
		
		if (bDOSFormat && (line.find('\n') != -1))
			line = line.erase(0, 1);
		
		int nTagStart = line.find('<');
		int nTagEnd = line.find('>');
		
		if (nTagStart > -1 && nTagEnd > nTagStart)
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
				pStatementTransactionResponse.reset(new OFXStatementTransactionResponse());				
			}
			else if (tag == "/STMTTRNRS" || tag == "/CCSTMTTRNRS")
			{
				dataItem.addStatementTransactionResponse(pStatementTransactionResponse);
			}
			else if (tag == "STMTRS" || tag == "CCSTMTRS")
			{
				pStatementResponse.reset(new OFXStatementResponse());
			}
			else if (tag == "/STMTRS" || tag  == "/CCSTMTRS")
			{
				pStatementTransactionResponse->setStatementResponse(pStatementResponse);
			}
			else if (tag == "BANKACCTFROM")
			{
				pBankAccount.reset(new OFXBankAccount());
			}
			else if (tag == "CCACCTFROM")
			{
				pBankAccount.reset(new OFXBankAccount());
				
				pBankAccount->setType(CreditCard);
			}
			else if (tag == "BANKID")
			{
				pBankAccount->setBankID(data);
			}
			else if (tag == "ACCTID")
			{
				pBankAccount->setAccountID(data);
			}
			else if (tag == "ACCTTYPE")
			{
				pBankAccount->setType(data);
			}
			else if (tag == "/BANKACCTFROM" || tag == "/CCACCTFROM")
			{
				pStatementResponse->setAccount(pBankAccount);
			}
			else if (tag == "STMTTRN")
			{
				pTransaction.reset(new OFXStatementTransaction());
			}
			else if (tag == "TRNTYPE")
			{
				pTransaction->setType(data);
			}
			else if (tag == "DTPOSTED")
			{
				pTransaction->setDate(data);
			}
			else if (tag == "TRNAMT")
			{
				pTransaction->setAmount(data);
			}
			else if (tag == "NAME")
			{
				pTransaction->setName(data);
			}
			else if (tag == "MEMO")
			{
				pTransaction->setMemo(data);
			}
			else if (tag == "FITID")
			{
				pTransaction->setFITID(data);
			}
			else if (tag == "/STMTTRN")
			{
				pStatementResponse->addTransaction(pTransaction);
			}
			else if (tag == "BALAMT")
			{
				pStatementResponse->setBalance(data);
			}
		}		
	}
	
	fileStream.close();

	return true;
}

bool importOFXStatementIntoAccount(Account &account, OFXStatementResponse &statement, bool reverseTransactions, bool reconciled, bool ignoreDuplicates)
{
	if (reverseTransactions)
	{
		statement.reverseTransactionOrder();
	}
	
	OFXStTrIt transactionsIt = statement.begin();
	OFXStTrIt transactionsItEnd = statement.end();
	
	bool ignoreThisOne = false;
	
	for (; transactionsIt != transactionsItEnd; ++transactionsIt)
	{
		OFXStatementTransaction itemTrans = (*transactionsIt);
		
		Transaction newTransaction(itemTrans.getMemo(), itemTrans.getName(), "", itemTrans.getAmount(), itemTrans.getDate());
		newTransaction.setType(itemTrans.getType());
		
		if (reconciled)
		{
			newTransaction.setReconciled(true);
		}
		
		ignoreThisOne = false;
		
		if (!itemTrans.getFITID().empty()) // if there's a FITID, apply it to the Transaction so it will get saved
		{
			const std::string strFITID = itemTrans.getFITID();
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
