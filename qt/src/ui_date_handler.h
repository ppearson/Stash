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

#ifndef UI_DATE_HANDLER_H
#define UI_DATE_HANDLER_H

#include <QString>
#include <QLocale>
#include <QDate>

#include "../../core/date.h"

class UIDateHandler
{
public:
	UIDateHandler()
	{
	}

	virtual ~UIDateHandler()
	{
	}
	
	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatDate(const Date& date) = 0;
	
	virtual QString getDatePickerDisplayFormat() const = 0;
	
};

class UIDateHandler_DMY : public UIDateHandler
{
public:
	UIDateHandler_DMY()
	{
	}

	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatDate(const Date& date)
	{
		return QString(date.FormattedDate(Date::UK).c_str());
	}
	
	virtual QString getDatePickerDisplayFormat() const
	{
		return QString("dd/MM/yyyy");
	}
};

class UIDateHandler_MDY : public UIDateHandler
{
public:
	UIDateHandler_MDY()
	{
	}

	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatDate(const Date& date)
	{
		return QString(date.FormattedDate(Date::US).c_str());
	}
	
	virtual QString getDatePickerDisplayFormat() const
	{
		return QString("MM/dd/yyyy");
	}	
};

class UIDateHandler_ISO8601 : public UIDateHandler
{
public:
	UIDateHandler_ISO8601()
	{
	}

	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatDate(const Date& date)
	{
		// from temp?!
		return QString(date.FormattedDate(Date::ISO_8601).c_str());
	}
	
	virtual QString getDatePickerDisplayFormat() const
	{
		return QString("yyyy-MM-dd");
	}	
};

class UIDateHandler_QLocale : public UIDateHandler
{
public:
	UIDateHandler_QLocale()
	{
		m_dateFormat = m_locale.dateFormat(QLocale::ShortFormat);
	}

	// this is non-const on purpose, as derived classes may
	// have an internal cache/buffer...
	virtual QString formatDate(const Date& date)
	{
		QDate tmpDate(date.getYear(), date.getMonth(), date.getDay());
		return tmpDate.toString(m_dateFormat);
	}
	
	virtual QString getDatePickerDisplayFormat() const
	{
		return m_dateFormat;
	}
	
protected:
	QLocale		m_locale;
	
	QString		m_dateFormat;
};

#endif // UI_DATE_HANDLER_H

