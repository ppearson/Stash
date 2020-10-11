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

#ifndef NEW_GRAPH_DIALOG_H
#define NEW_GRAPH_DIALOG_H

#include <string>

#include <QDialog>
#include <QDialogButtonBox>

#include "../../core/datetime.h"

class QLineEdit;
class QComboBox;
class QCheckBox;

class Document;

class NewGraphDialog : public QDialog
{
	Q_OBJECT
public:
	NewGraphDialog(const Document& document, QWidget* parent);
	
	
	
	const std::string& getName() const
	{
		return m_name;
	}
	
	int getAccountIndex() const
	{
		return m_accountIndex;
	}
	
	bool getIgnoreTransfers() const
	{
		return m_ignoreTransfers;
	}
	
	const Date& getStartDate() const
	{
		return m_startDate;
	}
	
	const Date& getEndDate() const
	{
		return m_endDate;
	}
	
public slots:
	void OKClicked();
	
protected:
	const Document&	m_document;
	
	// widgets
	QDialogButtonBox*	m_pButtonBox;
	
	QLineEdit*			m_pName;
	QComboBox*			m_pAccount;
	QCheckBox*			m_pIgnoreTransfers;
	QComboBox*			m_pDatesPreset;
	
	// values
	
	std::string			m_name;
	int					m_accountIndex;
	bool				m_ignoreTransfers;
	Date				m_startDate;
	Date				m_endDate;
};

#endif // NEW_GRAPH_DIALOG_H
