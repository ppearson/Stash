/*
 * Stash:  A Personal Finance app (Qt UI).
 * Copyright (C) 2020-2021 Peter Pearson
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

#ifndef UI_CURRENCY_HANDLER_H
#define UI_CURRENCY_HANDLER_H

#include <QString>
#include <QLocale>

#include "../../core/currency_value_formatter.h"
#include "../../core/fixed.h"

class UICurrencyHandler
{
public:
	UICurrencyHandler()
	{
	}

	virtual ~UICurrencyHandler()
	{
	}
	
	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatCurrencyAmount(const fixed& amount) = 0;
	
	virtual QChar getThousandsSeparatorChar() const = 0;
	
	virtual bool parseStringAmountValue(const QString& stringAmount, double& doubleAmount) const
	{
		QChar thousandsSeparatorChar = getThousandsSeparatorChar();
		
		QString simplifiedString;
		int length = stringAmount.size();
		for (int i = 0; i < length; i++)
		{
			QChar chr = stringAmount.at(i);
			QChar::Category cat = chr.category();
			if (cat == QChar::Symbol_Currency)
			{
				continue;
			}
			
			if (chr == thousandsSeparatorChar ||
				chr == ' ')
			{
				continue;
			}
			
			simplifiedString += chr;
		}
		
		bool isOK = false;
		doubleAmount = simplifiedString.toDouble(&isOK);
		return isOK;
	}
};

class UICurrHandler_DollarNegSymbolPrefix : public UICurrencyHandler
{
public:
	UICurrHandler_DollarNegSymbolPrefix()
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

class UICurrHandler_DollarNegParenthesis : public UICurrencyHandler
{
public:
	UICurrHandler_DollarNegParenthesis()
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

class UICurrHandler_PoundSterling : public UICurrencyHandler
{
public:
	UICurrHandler_PoundSterling()
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
class UICurrHandler_QLocale : public UICurrencyHandler
{
public:
	UICurrHandler_QLocale()
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

#endif // UI_CURRENCY_HANDLER_H
