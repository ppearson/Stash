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

#include "transactions_view_widget.h"

#include <QTreeView>
#include <QLayout>
#include <QHeaderView>
#include <QSplitter>

#include "../../core/account.h"
#include "../../core/transaction.h"
#include "../../core/split_transaction.h"

#include "form_panels/transaction_form_panel.h"

#include "transactions_view_data_model.h"

static const std::string sTableStyle = "QTreeView::item {"
		"border: 0.5px solid #8c8c8c;"
        "border-top-color: transparent;"
        "border-left-color: transparent;"
		"}"		
		"QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};";

TransactionsViewWidget::TransactionsViewWidget(QWidget* pParent) : QWidget(pParent),
    m_pAccount(nullptr),
	m_pSplitter(nullptr),
    m_pTreeView(nullptr),
    m_pModel(nullptr),
	m_transactionIndex(-1),
	m_splitTransactionIndex(-1)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);

	setLayout(layout);
	
	//
	
	m_pSplitter = new QSplitter(Qt::Vertical, this);
	m_pSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	layout->addWidget(m_pSplitter);
	
	m_pTreeView = new QTreeView(m_pSplitter);
	m_pTreeView->setHeaderHidden(false);
//	m_pTreeView->setFrameStyle(QFrame::NoFrame);
	m_pTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pTreeView->setAttribute(Qt::WA_MacShowFocusRect, false); // hide the OS X blue focus rect

	m_pModel = new TransactionsViewDataModel(this);

	m_pTreeView->setModel(m_pModel);
	
	m_pTreeView->setColumnWidth(0, 120);
	m_pTreeView->setColumnWidth(1, 150);
	m_pTreeView->setColumnWidth(2, 170);
	m_pTreeView->setColumnWidth(3, 520);
	m_pTreeView->setColumnWidth(4, 120);
	m_pTreeView->setColumnWidth(5, 140);
	
	m_pTreeView->setAlternatingRowColors(true);
	
	m_pTreeView->setStyleSheet(sTableStyle.c_str());
	
	m_pTransactionFormPanel = new TransactionFormPanel(m_pSplitter);
	
	m_pSplitter->addWidget(m_pTreeView);
	m_pSplitter->addWidget(m_pTransactionFormPanel);

	m_pSplitter->setHandleWidth(2);	
	m_pSplitter->setStretchFactor(0, 2);
	m_pSplitter->setStretchFactor(1, 0);
	
	//
	
//	m_pModel->rebuildModelFromDocument();


	connect(m_pTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
			SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	
	connect(m_pTransactionFormPanel, SIGNAL(transactionValuesUpdated()), this, SLOT(transactionValuesUpdated()));
}

QSize TransactionsViewWidget::minimumSizeHint() const
{
	return QSize(10, 10);
}

QSize TransactionsViewWidget::sizeHint() const
{
	return QSize(600, 300);
}


void TransactionsViewWidget::rebuildFromAccount()
{
	m_pModel->setAccount(m_pAccount);
		
	m_pModel->rebuildModelFromAccount();
	
	// resetting the model doesn't reset the scroll position of the QTreeView, so we need to do that manually...
	
	m_pTreeView->scrollToTop();
	
	m_pTransactionFormPanel->clear(true);
	
	m_transactionIndex = -1;
	m_splitTransactionIndex = -1;
}

void TransactionsViewWidget::setViewDurationType(TransactionsViewDurationType viewType)
{
	m_pModel->setTransactionsViewDurationType(viewType);
	
	m_pTreeView->scrollToTop();
	
	m_pTransactionFormPanel->clear(true);
	
	m_transactionIndex = -1;
	m_splitTransactionIndex = -1;
}

void TransactionsViewWidget::addNewTransaction()
{
	if (!m_pAccount)
		return;
	
	Date currentlyEnteredDate = m_pTransactionFormPanel->getEnteredDate();
	
	Transaction startingTransaction("", "", "", fixed(0.0), currentlyEnteredDate);
	startingTransaction.setCleared(true);
	
	int nextTransactionIndex = m_pModel->rowCount();
	m_pAccount->addTransaction(startingTransaction);
	
	// TODO: use insertRow() rather than brute-force rebuild...
	m_pModel->rebuildModelFromAccount();
	
	selectTransaction(nextTransactionIndex);
	
	m_pTransactionFormPanel->setFocusPayee();
}

void TransactionsViewWidget::splitCurrentTransaction()
{
	if (!m_pAccount)
		return;
	
	Transaction& transaction = m_pAccount->getTransaction(m_transactionIndex);
	
	transaction.setSplit(true);
	
	QModelIndex newlySplitTransactionIndex = m_pTreeView->selectionModel()->currentIndex();
	
	// save the row index to construct a new QModelIndex after we've refreshed it.
	int modelRowIndex = newlySplitTransactionIndex.row();	
	
	m_pModel->rebuildModelFromAccount();
	
	newlySplitTransactionIndex = m_pTreeView->model()->index(modelRowIndex, 0);
			
	m_pTreeView->expand(newlySplitTransactionIndex);
		
	// now select first child
	
	QModelIndex newChildIndex = newlySplitTransactionIndex.child(0, 0);
	
	m_pTreeView->selectionModel()->select(newChildIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	m_pTreeView->scrollTo(newChildIndex);
}

void TransactionsViewWidget::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	if (!m_pAccount)
		return;
	
	QList<QModelIndex> indexes = selected.indexes();
	// just select the first for now

	if (indexes.isEmpty())
		return;

	QModelIndex& index = indexes[0];
	if (!index.isValid())
		return;
	
	TransactionsModelItem* item = static_cast<TransactionsModelItem*>(index.internalPointer());
	if (item)
	{
		m_transactionIndex = item->getTransactionIndex();
		
		m_splitTransactionIndex = item->getSplitTransactionIndex();
		
		const Transaction& transaction = m_pAccount->getTransaction(m_transactionIndex);
		
		if (m_splitTransactionIndex == -1)
		{
			// it's not a split transaction, so just set the params directly from the transaction
			m_pTransactionFormPanel->setParamsFromTransaction(transaction);
		}
		else if (m_splitTransactionIndex >= 0)
		{
			// it's an existing split transaction
			
			const SplitTransaction& splitTransaction = transaction.getSplit((unsigned int)m_splitTransactionIndex);
			m_pTransactionFormPanel->setParamsFromSplitTransaction(splitTransaction);
		}
		else if (m_splitTransactionIndex == -2)
		{
			// because remainder splits don't exist as an actual split transaction (they're virtual and UI-only),
			// we need to handle this case specifically...
			
			// because we haven't got an actual fixed or double amount anywhere, it's only in string form,
			// just use that for the moment...
			m_pTransactionFormPanel->setParamsForEmptySplitTransaction(item->getAmount().toString());
		}
	}
}

// called based on the "Update" button being pressed on the TransactionsFormPanel.
void TransactionsViewWidget::transactionValuesUpdated()
{
	if (!m_pAccount)
		return;
	
	if (m_transactionIndex == -1)
		return;
	
	Transaction& transaction = m_pAccount->getTransaction(m_transactionIndex);
	
	QModelIndex mainTransactionItemIndex;
	QModelIndex splitTransactionItemIndex;
	
	if (m_splitTransactionIndex == -1)
	{
		// it's not a split transaction, it's a normal one...
		
		m_pTransactionFormPanel->updateTransactionFromParamValues(transaction);
		
		mainTransactionItemIndex = m_pTreeView->selectionModel()->currentIndex();
	}
	else if (m_splitTransactionIndex >= 0)
	{
		// it's an existing split transaction that we're updating
		
		SplitTransaction& splitTransaction = transaction.getSplit((unsigned int)m_splitTransactionIndex);
		
		m_pTransactionFormPanel->updateSplitTransactionFromParamValues(splitTransaction);
		
		splitTransactionItemIndex = m_pTreeView->selectionModel()->currentIndex();
		mainTransactionItemIndex = splitTransactionItemIndex.parent();
	}
	else if (m_splitTransactionIndex == -2)
	{
		// it's a "remainder" non-existant split transaction, so we need to create a new one
		// to represent the values
		
		SplitTransaction newSplitTransaction;
		
		m_pTransactionFormPanel->updateSplitTransactionFromParamValues(newSplitTransaction);
		
		transaction.addSplit(newSplitTransaction);
		
		// TODO: this isn't working properly...
		splitTransactionItemIndex = m_pTreeView->selectionModel()->currentIndex();
		mainTransactionItemIndex = splitTransactionItemIndex.parent();
	}
	
	// save the row index to construct a new QModelIndex after we've refreshed it.
	int mainTransactionModelRowIndex = mainTransactionItemIndex.row();
	int splitTransactionModelRowIndex = splitTransactionItemIndex.isValid() ? splitTransactionItemIndex.row() : -1;
	
	// only a partial reset...
	// TODO: this is still pretty brute-force - would be nice to edit-in-place just the single item...
	m_pModel->rebuildModelFromAccount();
	
	mainTransactionItemIndex = m_pTreeView->model()->index(mainTransactionModelRowIndex, 0);
	
	if (m_splitTransactionIndex == -1)
	{
		// reselect the item
		m_pTreeView->selectionModel()->select(mainTransactionItemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		m_pTreeView->scrollTo(mainTransactionItemIndex);
	}
	else
	{
		// expand the main transaction
		m_pTreeView->expand(mainTransactionItemIndex);
		
		// if the split still has a remainder, increment the split index, so we end up
		// automatically selecting the remainder item...
		if (transaction.getSplitTotal() > transaction.getAmount())
		{
			splitTransactionModelRowIndex++;
		}
		
		splitTransactionItemIndex = mainTransactionItemIndex.child(splitTransactionModelRowIndex, 0);
		
		m_pTreeView->selectionModel()->select(splitTransactionItemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		m_pTreeView->scrollTo(splitTransactionItemIndex);
	}
}

void TransactionsViewWidget::selectTransaction(unsigned int transactionIndex, int splitIndex)
{
	QModelIndex selectionIndex = m_pTreeView->model()->index(transactionIndex, 0);
	m_pTreeView->selectionModel()->select(selectionIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	m_pTreeView->scrollTo(selectionIndex);
}
