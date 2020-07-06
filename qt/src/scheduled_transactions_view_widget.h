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

#ifndef SCHEDULED_TRANSACTIONS_VIEW_WIDGET_H
#define SCHEDULED_TRANSACTIONS_VIEW_WIDGET_H

#include <QWidget>
#include <QModelIndex>

//#include "view_common.h"

class QTreeView;
class QItemSelection;
class QSplitter;

class Document;
class ScheduledTransactionsViewDataModel;
class ScheduledTransactionFormPanel;
class ItemControlButtonsWidget;

class StashWindow;

class ScheduledTransactionsViewWidget : public QWidget
{
	Q_OBJECT
public:
	ScheduledTransactionsViewWidget(Document& document, QWidget* pParent, StashWindow* mainWindow);
	
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
	void rebuildFromDocument();
	
	void addNewScheduledTransaction();
	void deleteSelectedScheduledTransaction();

signals:

public slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	
	void scheduledTransactionValuesUpdated();
	
	void addItemClicked();
	void deleteItemClicked();
	
protected:
	void selectScheduledTransaction(unsigned int scheduledTransactionIndex);
	
	// helper to conveniently get single selected item
	QModelIndex getSingleSelectedIndex() const;
	
	void updateItemButtonsFromSelection();
	
	
	
protected:
	StashWindow*						m_pMainWindow;
	
	Document&							m_document;
	
	QSplitter*							m_pSplitter;
	
	QTreeView*							m_pTreeView;
	ScheduledTransactionsViewDataModel*	m_pModel;
	
	ScheduledTransactionFormPanel*		m_pScheduledTransactionFormPanel;
	
	ItemControlButtonsWidget*			m_pItemControlButtons;
	
	
	// Cached indices of scheduled transaction selection. These map into the
	// Document/ScheduledTransaction, *not* the DataModel...
	unsigned int						m_scheduledTransactionIndex;
};

#endif // SCHEDULED_TRANSACTIONS_VIEW_WIDGET_H
