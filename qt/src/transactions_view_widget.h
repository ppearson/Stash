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

#ifndef TRANSACTIONS_VIEW_WIDGET_H
#define TRANSACTIONS_VIEW_WIDGET_H

#include <QWidget>
#include <QModelIndex>

#include "view_common.h"

class QTreeView;
class QItemSelection;
class QSplitter;

class Account;
class TransactionsViewDataModel;
class TransactionFormPanel;
class ItemControlButtonsWidget;

class StashWindow;

class TransactionsViewWidget : public QWidget
{
	Q_OBJECT
public:
	TransactionsViewWidget(QWidget* pParent, StashWindow* mainWindow);
	
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
	void setAccount(Account* pAccount)
	{
		m_pAccount = pAccount;
		m_transactionIndex = -1;
	}

	void rebuildFromAccount(bool refreshDatePickerFormat = false);
	
	void setViewDurationType(TransactionsViewDurationType viewType);
	
	void addNewTransaction();
	void deleteSelectedTransaction();
	void splitCurrentTransaction();
	
	void moveSelectedTransactionUp();
	void moveSelectedTransactionDown();
	
	// if ignoreSetting == false, then use the global setting,
	// otherwise, always scroll to last one.
	void scrollToLastTransaction(bool ignoreSetting = false);
	
signals:

public slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	
	void transactionValuesUpdated();
	
	void addItemClicked();
	void deleteItemClicked();
	void splitItemClicked();
	void moveUpItemClicked();
	void moveDownItemClicked();

	void transactionClearedStateToggled(unsigned int transactionIndex, bool newClearedState);
	
protected:
	void selectTransaction(unsigned int transactionIndex, int splitIndex = -1);
	
	// helper to conveniently get single selected item
	QModelIndex getSingleSelectedIndex() const;
	
	void updateItemButtonsAndMainMenuStateFromSelection();
	
protected:
	StashWindow*				m_pMainWindow;
	
	Account*					m_pAccount;
	
	QSplitter*					m_pSplitter;
	
	QTreeView*					m_pTreeView;
	TransactionsViewDataModel*	m_pModel;
	
	TransactionFormPanel*		m_pTransactionFormPanel;
	
	ItemControlButtonsWidget*	m_pItemControlButtons;
	
	
	// Cached indices of transaction selection. These map into the Account/Transaction, *not* the DataModel...
	unsigned int				m_transactionIndex;
	int							m_splitTransactionIndex;
	
};

#endif // TRANSACTIONS_VIEW_WIDGET_H
