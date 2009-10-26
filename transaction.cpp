#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <bitset>
#include "transaction.h"
#include "string.h"

Transaction::Transaction(std::string Description, std::string Payee, std::string Category, fixed Amount, Date date) :
	m_Description(Description), m_Payee(Payee), m_Category(Category), m_Amount(Amount), m_Split(false), m_Reconciled(false),
	m_Type(None)
{
	m_Date.Now();
}

void Transaction::Load(std::fstream &stream, int version)
{
	m_Date.Load(stream, version);
	LoadString(m_Description, stream);
	LoadString(m_Payee, stream);
	LoadString(m_Category, stream);
	m_Amount.Load(stream, version);
	
	stream.read((char *) &m_Type, sizeof(TransactionType));
	
	unsigned char cBitset = 0;
	stream.read((char *) &cBitset, sizeof(unsigned char));
	
	std::bitset<8> localset(static_cast<unsigned long>(cBitset));
		
	m_Reconciled = localset[0];
	m_Split = localset[1];
	
	unsigned char numSplits = 0;
	
	stream.read((char *) &numSplits, sizeof(unsigned char));
		
	for (int i = 0; i < numSplits; i++)
	{
		SplitTransaction tempSplit;
		tempSplit.Load(stream, version);
		
		m_aSplits.push_back(tempSplit);
	}
}

void Transaction::Store(std::fstream &stream)
{
	m_Date.Store(stream);
	StoreString(m_Description, stream);
	StoreString(m_Payee, stream);
	StoreString(m_Category, stream);
	m_Amount.Store(stream);
	
	stream.write((char *) &m_Type, sizeof(TransactionType));
	
	std::bitset<8> localset;	
	localset[0] = m_Reconciled;
	localset[1] = m_Split;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	stream.write((char *) &cBitset, sizeof(unsigned char));
	
	unsigned char numSplits = static_cast<unsigned char>(m_aSplits.size());
	stream.write((char *) &numSplits, sizeof(unsigned char));
	
	for (std::vector<SplitTransaction>::iterator it = m_aSplits.begin(); it != m_aSplits.end(); ++it)
	{
		(*it).Store(stream);
	}
}

void Transaction::addSplit(std::string Description, std::string Payee, std::string Category, fixed Amount)
{
	SplitTransaction split(Description, Payee, Category, Amount);
	
	m_aSplits.push_back(split);
	m_Split = true;
}

fixed Transaction::getSplitTotal()
{
	fixed total = 0.0;
	
	for (std::vector<SplitTransaction>::iterator it = m_aSplits.begin(); it != m_aSplits.end(); ++it)
	{
		total += (*it).Amount();
	}
	
	return total;	
}


