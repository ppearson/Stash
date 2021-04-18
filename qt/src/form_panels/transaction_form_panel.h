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

#ifndef TRANSACTION_FORM_PANEL_H
#define TRANSACTION_FORM_PANEL_H

#include <string>

#include <QWidget>

#include "../../core/date.h"
#include "../../core/transaction.h"

class Document;
class StashWindow;

class QLineEdit;
class QComboBox;
class QCheckBox;
class QDateEdit;

class ExpressionLineEdit;

class TransactionFormPanel : public QWidget
{
	Q_OBJECT
public:
	TransactionFormPanel(const StashWindow* pStashWindow, Document& document, QWidget* parent = 0);
	
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
	void clear(bool resetDate);
	void setFocusPayee(bool selectText = false);
	
	void setParamsFromTransaction(const Transaction& transaction);
	void setParamsFromSplitTransaction(const SplitTransaction& splitTransaction);
	void setParamsForEmptySplitTransaction(QString amountString);

	void setClearedParamValue(bool clearedValue);
	
	void updateTransactionFromParamValues(Transaction& transaction);
	void updateSplitTransactionFromParamValues(SplitTransaction& splitTransaction);
	
	
	// get hold of the currently-entered date for when adding new transactions
	Date getEnteredDate() const;
	
	virtual void keyPressEvent(QKeyEvent* event);
	
	
signals:
	void transactionValuesUpdated();

public slots:
	void updateClicked();
	
	
protected:
	void updatePayeeAndCategoryComboBoxChoicesFromDocument();
	
protected:
	const StashWindow*	m_pStashWindow;
	Document&			m_document;
	
	QComboBox*			m_pPayee;
	ExpressionLineEdit*	m_pAmount;
	QComboBox*			m_pCategory;
	QCheckBox*			m_pCleared;
	QComboBox*			m_pType;
	QDateEdit*			m_pDate;
	QLineEdit*			m_pDescription;
	
};

#endif // TRANSACTION_FORM_PANEL_H
