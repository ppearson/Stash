/* 
 * Stash:  A Personal Finance app.
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

#include "currency_value_formatter.h"

#include <cstring>
#include <iostream>

#include "fixed.h"

CurrencyValueFormatter::CurrencyValueFormatter()
{
	setFromPreset(ePresetNZ);
}

CurrencyValueFormatter::CurrencyValueFormatter(Preset preset)
{
	setFromPreset(preset);
}

CurrencyValueFormatter::~CurrencyValueFormatter()
{
	if (m_pImpl)
	{
		delete m_pImpl;
		m_pImpl = nullptr;
	}
}

void CurrencyValueFormatter::setFromPreset(Preset preset)
{
	if (m_pImpl)
	{
		delete m_pImpl;
		m_pImpl = nullptr;
	}
	
	if (preset == ePresetNZ)
	{
		m_pImpl = new IFImpl_DollarNegSymbolPrefix();
	}
	else if (preset == ePresetUK)
	{
		m_pImpl = new IFImpl_PoundSterling();
	}
	else if (preset == ePresetUS)
	{
		m_pImpl = new IFImpl_DollarNegParenthesis();
	}
}

const char* CurrencyValueFormatter::IFImpl_DollarNegSymbolPrefix::formatValue(const fixed& value)
{
	// try and do everything in one pass (although with an extra buffer)
	// without reversing anything...
	unsigned int pos = 0;
	if (!value.IsPositive())
	{
		m_buffer[pos++] = '-';
	}
	m_buffer[pos++] = '$';
	
	// TODO: we could probably do this without the extra buffer and sprintf() step, but...
	
	sprintf(m_tempBuffer, "%0.2f", value.ToDoubleAbs());
	
	unsigned int numBaseDigits = value.GetNumDigits();
	
	const char* pSrcBuffer = (const char*)&m_tempBuffer[0];
	
	bool addSepNext = false;
	
	while (*pSrcBuffer != 0)
	{
		if (addSepNext)
		{
			m_buffer[pos++] = ',';
		}
		
		if (numBaseDigits > 3 && --numBaseDigits % 3 == 0)
		{
			addSepNext = true;
		}
		else
		{
			addSepNext = false;
		}
			
		m_buffer[pos++] = *pSrcBuffer;		
		
		pSrcBuffer ++;
	}
	
	m_buffer[pos] = 0;
	
	return &m_buffer[0];
}

const char* CurrencyValueFormatter::IFImpl_PoundSterling::formatValue(const fixed& value)
{
	// try and do everything in one pass (although with an extra buffer)
	// without reversing anything...
	
	m_buffer.clear();
		
	if (!value.IsPositive())
	{
		m_buffer += '-';
	}
	m_buffer += u8"£";
	
	// TODO: we could probably do this without the extra buffer and sprintf() step, but...
	
	sprintf(m_tempBuffer, "%0.2f", value.ToDoubleAbs());
	
	unsigned int numBaseDigits = value.GetNumDigits();
	
	const char* pSrcBuffer = (const char*)&m_tempBuffer[0];
	
	bool addSepNext = false;
	
	while (*pSrcBuffer != 0)
	{
		if (addSepNext)
		{
			m_buffer += ',';
		}
		
		if (numBaseDigits > 3 && --numBaseDigits % 3 == 0)
		{
			addSepNext = true;
		}
		else
		{
			addSepNext = false;
		}
			
		m_buffer += *pSrcBuffer;		
		
		pSrcBuffer ++;
	}
		
	return m_buffer.c_str();
}

const char* CurrencyValueFormatter::IFImpl_DollarNegParenthesis::formatValue(const fixed& value)
{
	// try and do everything in one pass (although with an extra buffer)
	// without reversing anything...
	unsigned int pos = 0;
	if (!value.IsPositive())
	{
		m_buffer[pos++] = '(';
	}
	m_buffer[pos++] = '$';
	
	// TODO: we could probably do this without the extra buffer and sprintf() step, but...
	
	sprintf(m_tempBuffer, "%0.2f", value.ToDoubleAbs());
	
	unsigned int numBaseDigits = value.GetNumDigits();
	
	const char* pSrcBuffer = (const char*)&m_tempBuffer[0];
	
	bool addSepNext = false;
	
	while (*pSrcBuffer != 0)
	{
		if (addSepNext)
		{
			m_buffer[pos++] = ',';
		}
		
		if (numBaseDigits > 3 && --numBaseDigits % 3 == 0)
		{
			addSepNext = true;
		}
		else
		{
			addSepNext = false;
		}
			
		m_buffer[pos++] = *pSrcBuffer;		
		
		pSrcBuffer ++;
	}
	
	if (!value.IsPositive())
	{
		m_buffer[pos++] = ')';
	}
	m_buffer[pos] = 0;
	
	return &m_buffer[0];
}



