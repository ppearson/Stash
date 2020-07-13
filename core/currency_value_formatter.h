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

#ifndef CURRENCY_VALUE_FORMATTER_H
#define CURRENCY_VALUE_FORMATTER_H

#include <string>

class fixed;

// Would have preferred not to have to bother with this type of stuff, but Qt's QLocale
// currency formatting is apparently pretty poor, so we have to do it ourselves in order
// to get negative symbols in the right place (*before* the currency symbol for NZ/UK,
// Qt puts it *after*), and to get thousands grouping which QLocale does not seem to do...

class CurrencyValueFormatter
{
public:
	enum Preset
	{
		ePresetNZ,
		ePresetUK,
		ePresetUS
	};

	// TODO: find some 'nice' way of automatically detecting
	//       the system locale, and extracting details from QLocale
	//       for the Qt UI side of things.
	CurrencyValueFormatter();
	CurrencyValueFormatter(Preset preset);
	
	~CurrencyValueFormatter();
	
	void setFromPreset(Preset preset);
	
	const char* formatValue(const fixed& value)
	{
		return m_pImpl->formatValue(value);
	}	
	
private:
	class InternalFormatterImplementation
	{
	public:
		InternalFormatterImplementation()
		{	
		}
		
		virtual ~InternalFormatterImplementation()
		{
		}
		
		virtual const char* formatValue(const fixed& value) = 0;
	};
	
	//
	class IFImpl_DollarNegSymbolPrefix : public InternalFormatterImplementation
	{
	public:
		IFImpl_DollarNegSymbolPrefix()
		{
		}
		
		virtual const char* formatValue(const fixed& value) override;
		
	protected:
		char		m_buffer[32];    // should be more than enough! :)
		char		m_tempBuffer[32];
	};
	
	class IFImpl_PoundSterling : public InternalFormatterImplementation
	{
	public:
		IFImpl_PoundSterling()
		{
		}
		
		virtual const char* formatValue(const fixed& value) override;
		
	protected:
		std::string	m_buffer;
		char		m_tempBuffer[32];
	};
	
	class IFImpl_DollarNegParenthesis : public InternalFormatterImplementation
	{
	public:
		IFImpl_DollarNegParenthesis()
		{
		}
		
		virtual const char* formatValue(const fixed& value) override;
		
	protected:
		char		m_buffer[32];    // should be more than enough! :)
		char		m_tempBuffer[32];
	};
	
	
private:
	InternalFormatterImplementation*	m_pImpl = nullptr;
};

#endif // CURRENCY_VALUE_FORMATTER_H
