/*
 * Stash:  A Personal Finance app (Qt UI).
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

#ifndef UI_CURRENCY_FORMATTER_H
#define UI_CURRENCY_FORMATTER_H

#include <QString>
#include <QLocale>

#include "../../core/currency_value_formatter.h"
#include "../../core/fixed.h"

class UICurrencyFormatter
{
public:
	UICurrencyFormatter()
	{
		
	}

	virtual ~UICurrencyFormatter()
	{
		
	}
	
	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatCurrencyAmount(const fixed& amount) = 0;
	
	virtual QChar getThousandsSeparatorChar() const = 0;
};

class UICurrForm_DollarNegSymbolPrefix : public UICurrencyFormatter
{
public:
	UICurrForm_DollarNegSymbolPrefix()
	{
		m_formatter.setFromPreset(CurrencyValueFormatter::ePresetNZ);
	}
	
	virtual QString formatCurrencyAmount(const fixed& amount)
	{
		return QString::fromUtf8(m_formatter.formatValue(amount));
	}
	
	virtual QChar getThousandsSeparatorChar() const
	{
		return m_formatter.getThousandsSeparatorChar();
	}
	
protected:
	CurrencyValueFormatter	m_formatter;
};

class UICurrForm_DollarNegParenthesis : public UICurrencyFormatter
{
public:
	UICurrForm_DollarNegParenthesis()
	{
		m_formatter.setFromPreset(CurrencyValueFormatter::ePresetUS);
	}
	
	virtual QString formatCurrencyAmount(const fixed& amount)
	{
		return QString::fromUtf8(m_formatter.formatValue(amount));
	}
	
	virtual QChar getThousandsSeparatorChar() const
	{
		return m_formatter.getThousandsSeparatorChar();
	}
	
protected:
	CurrencyValueFormatter	m_formatter;
};

class UICurrForm_PoundSterling : public UICurrencyFormatter
{
public:
	UICurrForm_PoundSterling()
	{
		m_formatter.setFromPreset(CurrencyValueFormatter::ePresetUK);
	}
	
	virtual QString formatCurrencyAmount(const fixed& amount)
	{
		return QString::fromUtf8(m_formatter.formatValue(amount));
	}
	
	virtual QChar getThousandsSeparatorChar() const
	{
		return m_formatter.getThousandsSeparatorChar();
	}
	
protected:
	CurrencyValueFormatter	m_formatter;
};

// backup one for all else...
class UICurrForm_QLocale : public UICurrencyFormatter
{
public:
	UICurrForm_QLocale()
	{
		
	}
	
	virtual QString formatCurrencyAmount(const fixed& amount)
	{
		return m_locale.toCurrencyString(amount.ToDouble());
	}
	
	virtual QChar getThousandsSeparatorChar() const
	{
		return m_locale.groupSeparator();
	}
	
protected:
	QLocale		m_locale;
};

#endif // UI_CURRENCY_FORMATTER_H
