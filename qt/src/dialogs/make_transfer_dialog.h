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

#ifndef MAKE_TRANSFER_DIALOG_H
#define MAKE_TRANSFER_DIALOG_H

#include <QDialog>

#include <string>

#include <QDialogButtonBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QCheckBox>

#include "../../core/date.h"
#include "../../core/fixed.h"

class Document;

class StashWindow;

class MakeTransferDialog : public QDialog
{
	Q_OBJECT
public:
	MakeTransferDialog(const StashWindow* pStashWindow, const Document& document, QWidget* parent);
	
	// getters for data
	unsigned int getFromAccountIndex() const
	{
		return m_fromAccountIndex;
	}
	
	unsigned int getToAccountIndex() const
	{
		return m_toAccountIndex;
	}
	
	fixed getAmount() const
	{
		return m_amount;
	}
	
	const Date& getDate() const
	{
		return m_date;
	}
	
	const std::string& getCategory() const
	{
		return m_category;
	}
	
	const std::string& getDescription() const
	{
		return m_description;
	}
	
	bool getMarkTransactionsAsCleared() const
	{
		return m_markTransactionsAsCleared;
	}
	
signals:

public slots:
	void OKClicked();
	
private:
	const StashWindow* m_pStashWindow;
	const Document&	m_document;
	
	// widgets
	QDialogButtonBox*	m_pButtonBox;
	
	QComboBox*		m_pFromAccount;
	QComboBox*		m_pToAccount;
	
	QLineEdit*		m_pAmount;
	QDateEdit*		m_pDate;
	
	QComboBox*		m_pCategory;
	
	QLineEdit*		m_pDescription;
	
	QCheckBox*		m_pMarkTransactionsAsCleared;
	
	
	// data
	unsigned int	m_fromAccountIndex;
	unsigned int	m_toAccountIndex;
	
	fixed			m_amount;
	Date			m_date;
	
	std::string		m_category;
	std::string		m_description;
	
	bool			m_markTransactionsAsCleared;
};

#endif // MAKE_TRANSFER_DIALOG_H
