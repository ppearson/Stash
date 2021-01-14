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

#ifndef QIF_IMPORT_SETTINGS_DIALOG_H
#define QIF_IMPORT_SETTINGS_DIALOG_H

#include <string>

#include <QDialog>
#include <QDialogButtonBox>

#include "../../core/date.h"

class QLabel;
class QCheckBox;
class QComboBox;

class QIFImportSettingsDialog : public QDialog
{
	Q_OBJECT
public:
	QIFImportSettingsDialog(QWidget* parent, const std::string& fileName,
							const std::string& dateSample);
	
	
	Date::DateStringFormat getDateFormat() const
	{
		return m_dateFormat;
	}
	
	char getSeparator() const
	{
		return m_separator;
	}
	
	bool getMarkImportedTransactionsAsCleared() const
	{
		return m_markImportedTransactionsAsCleared;
	}
	
	
public slots:
	void OKClicked();
	
protected:
	// widgets
	QDialogButtonBox*	m_pButtonBox;
	
	QLabel*				m_pDescriptionLabel;
	QLabel*				m_pSampleDate;
	QComboBox*			m_pDateFormat;
	QCheckBox*			m_pMarkImportedTransactionsAsCleared;
	
	
	// cached values
	Date::DateStringFormat		m_dateFormat;
	char						m_separator;
	bool						m_markImportedTransactionsAsCleared;
};

#endif // QIF_IMPORT_SETTINGS_DIALOG_H
