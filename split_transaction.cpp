#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>

#include "split_transaction.h"
#include "string.h"

SplitTransaction::SplitTransaction(std::string Description, std::string Payee, fixed Amount) :
m_Description(Description), m_Payee(Payee), m_Amount(Amount)
{

}

void SplitTransaction::Load(std::fstream &stream)
{
	LoadString(m_Description, stream);
	LoadString(m_Payee, stream);
	m_Amount.Load(stream);	
}

void SplitTransaction::Store(std::fstream &stream)
{
	StoreString(m_Description, stream);
	StoreString(m_Payee, stream);
	m_Amount.Store(stream);
}
