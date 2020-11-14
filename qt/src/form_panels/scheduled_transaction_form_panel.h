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

#ifndef SCHEDULED_TRANSACTION_FORM_PANEL_H
#define SCHEDULED_TRANSACTION_FORM_PANEL_H

#include <string>

#include <QWidget>

#include "../../core/date.h"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QDateEdit;

class Document;
class ScheduledTransaction;

class ScheduledTransactionFormPanel : public QWidget
{
	Q_OBJECT
public:
	ScheduledTransactionFormPanel(const Document& document, QWidget* parent = 0);
	
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
	void clear();
	void setFocusPayee(bool selectText = false);
	
	void setParamsFromScheduledTransaction(const ScheduledTransaction& schedTransaction);
	
	void updateScheduledTransactionFromParamValues(ScheduledTransaction& schedTransaction);
	
	
	// get hold of the currently-entered date for when adding new transactions
	Date getEnteredDate() const;
	
	virtual void keyPressEvent(QKeyEvent* event);
	
protected:
	void updateAccountsList(bool selectFirst);
	void updatePayeeAndCategoryChoices();
	
signals:
	void scheduledTransactionValuesUpdated();

public slots:
	void updateClicked();
	
protected:
	const Document&		m_document;
		
	QComboBox*			m_pPayee;
	QComboBox*			m_pType;
	QComboBox*			m_pCategory;
	QComboBox*			m_pFrequency;
	QLineEdit*			m_pAmount;
	QComboBox*			m_pConstraint;
	QLineEdit*			m_pDescription;
	QDateEdit*			m_pDate;
	QComboBox*			m_pAccount;
	QCheckBox*			m_pEnabled;
};

#endif // SCHEDULED_TRANSACTION_FORM_PANEL_H
