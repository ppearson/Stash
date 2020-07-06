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

#include "scheduled_transactions_view_widget.h"

#include <QTreeView>
#include <QLayout>
#include <QHeaderView>
#include <QSplitter>

#include "../../core/document.h"
#include "../../core/scheduled_transaction.h"

#include "stash_window.h"

#include "form_panels/scheduled_transaction_form_panel.h"
#include "scheduled_transactions_view_data_model.h"

#include "item_control_buttons_widget.h"

static const std::string sTableStyle2 = "QTreeView::item {"
		"border: 0.5px solid #8c8c8c;"
        "border-top-color: transparent;"
        "border-left-color: transparent;"
		"}"		
		"QTreeView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};";

ScheduledTransactionsViewWidget::ScheduledTransactionsViewWidget(Document& document, QWidget* pParent, StashWindow* mainWindow) :
    QWidget(pParent),
	m_pMainWindow(mainWindow),
	m_document(document),
	m_pSplitter(nullptr),
    m_pTreeView(nullptr),
    m_pModel(nullptr),
	m_pScheduledTransactionFormPanel(nullptr),
	m_pItemControlButtons(nullptr),
	m_scheduledTransactionIndex(-1)
{
	QVBoxLayout* layout = new QVBoxLayout();
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

	m_pModel = new ScheduledTransactionsViewDataModel(m_pMainWindow, m_document, this);

	m_pTreeView->setModel(m_pModel);
	
	m_pTreeView->setColumnWidth(0, 100);
	m_pTreeView->setColumnWidth(1, 250);
	m_pTreeView->setColumnWidth(2, 250);
	m_pTreeView->setColumnWidth(3, 130);
	m_pTreeView->setColumnWidth(4, 190);
	m_pTreeView->setColumnWidth(5, 180);
	
	m_pTreeView->setAlternatingRowColors(true);
	
	m_pTreeView->setStyleSheet(sTableStyle2.c_str());
	
	m_pScheduledTransactionFormPanel = new ScheduledTransactionFormPanel(m_document, m_pSplitter);
	
	m_pSplitter->addWidget(m_pTreeView);
	m_pSplitter->addWidget(m_pScheduledTransactionFormPanel);

	m_pSplitter->setHandleWidth(2);	
	m_pSplitter->setStretchFactor(0, 2);
	m_pSplitter->setStretchFactor(1, 0);
	
	m_pItemControlButtons = new ItemControlButtonsWidget(ItemControlButtonsWidget::eScheduledTransaction, this);
	layout->addWidget(m_pItemControlButtons);

	connect(m_pTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
			SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	
	connect(m_pScheduledTransactionFormPanel, SIGNAL(scheduledTransactionValuesUpdated()), this, SLOT(scheduledTransactionValuesUpdated()));
	
	connect(m_pItemControlButtons, SIGNAL(addItemButtonClicked()), this, SLOT(addItemClicked()));
	connect(m_pItemControlButtons, SIGNAL(deleteItemButtonClicked()), this, SLOT(deleteItemClicked()));
	
	updateItemButtonsFromSelection();
}

QSize ScheduledTransactionsViewWidget::minimumSizeHint() const
{
	return QSize(10, 10);
}

QSize ScheduledTransactionsViewWidget::sizeHint() const
{
	return QSize(600, 300);
}

void ScheduledTransactionsViewWidget::rebuildFromDocument()
{
	m_pModel->rebuildModelFromDocument();
	
	// resetting the model doesn't reset the scroll position of the QTreeView, so we need to do that manually...
	
	m_pTreeView->scrollToTop();
	
	m_pScheduledTransactionFormPanel->clear();
	
	m_scheduledTransactionIndex = -1;
	
	updateItemButtonsFromSelection();
}

void ScheduledTransactionsViewWidget::addNewScheduledTransaction()
{
	// Not sure this is as useful for scheduled as it is for transactions, but...
	Date currentlyEnteredDate = m_pScheduledTransactionFormPanel->getEnteredDate();
	
	ScheduledTransaction startingScheduledTransaction;
	startingScheduledTransaction.setNextDate(currentlyEnteredDate);
	
	int nextScheduledTransactionIndex = m_pModel->rowCount();
	m_document.addScheduledTransaction(startingScheduledTransaction);
	
	// TODO: use insertRow() rather than brute-force rebuild...
	m_pModel->rebuildModelFromDocument();
	
	selectScheduledTransaction(nextScheduledTransactionIndex);
	
	m_pScheduledTransactionFormPanel->setFocusPayee();
	
	m_pMainWindow->setWindowModifiedAndRebuildIndex(true);
}

void ScheduledTransactionsViewWidget::deleteSelectedScheduledTransaction()
{
	if (m_scheduledTransactionIndex == -1)
		return;
	
	QModelIndex selectedModelScheduledTransactionIndex = getSingleSelectedIndex();
	
	m_document.deleteScheduledTransaction(m_scheduledTransactionIndex);
	
	// TODO: use removeRow in the model
	m_pModel->rebuildModelFromDocument();
	
	int deletedRow = selectedModelScheduledTransactionIndex.row();
	if (deletedRow < m_pModel->rowCount())
	{
		// select the next one down...
		QModelIndex newSelection = m_pModel->index(deletedRow, 0);
		m_pTreeView->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		m_pTreeView->scrollTo(newSelection);
	}
	
	m_pMainWindow->setWindowModifiedAndRebuildIndex(true);
}


void ScheduledTransactionsViewWidget::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QList<QModelIndex> indexes = selected.indexes();
	// just select the first for now

	if (indexes.isEmpty())
		return;

	QModelIndex& index = indexes[0];
	if (!index.isValid())
		return;
	
	ScheduledTransactionsModelItem* item = static_cast<ScheduledTransactionsModelItem*>(index.internalPointer());
	if (item)
	{
		m_scheduledTransactionIndex = item->getScheduledTransactionIndex();

		const ScheduledTransaction& schedTransaction = m_document.getScheduledTransaction(m_scheduledTransactionIndex);
		
		m_pScheduledTransactionFormPanel->setParamsFromScheduledTransaction(schedTransaction);
		
		updateItemButtonsFromSelection();
	}
}

// called based on the "Update" button being pressed on the ScheduledTransactionsFormPanel.
void ScheduledTransactionsViewWidget::scheduledTransactionValuesUpdated()
{
	if (m_scheduledTransactionIndex == -1)
		return;
	
	ScheduledTransaction& scheduledTransaction = m_document.getScheduledTransaction(m_scheduledTransactionIndex);
	
	QModelIndex scheduledTransactionItemIndex = getSingleSelectedIndex();
	
	m_pScheduledTransactionFormPanel->updateScheduledTransactionFromParamValues(scheduledTransaction);
	
	// save the row index to construct a new QModelIndex after we've refreshed it.
	int scheduledTransactionModelRowIndex = scheduledTransactionItemIndex.row();

	
	// only a partial reset...
	// TODO: this is still pretty brute-force - would be nice to edit-in-place just the single item...
	m_pModel->rebuildModelFromDocument();
	
	scheduledTransactionItemIndex = m_pTreeView->model()->index(scheduledTransactionModelRowIndex, 0);

	// reselect the item
	m_pTreeView->selectionModel()->select(scheduledTransactionItemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	m_pTreeView->scrollTo(scheduledTransactionItemIndex);

	m_pMainWindow->setWindowModifiedAndRebuildIndex(false);
}

void ScheduledTransactionsViewWidget::addItemClicked()
{
	addNewScheduledTransaction();
}

void ScheduledTransactionsViewWidget::deleteItemClicked()
{
	deleteSelectedScheduledTransaction();
}

void ScheduledTransactionsViewWidget::selectScheduledTransaction(unsigned int scheduledTransactionIndex)
{
	QModelIndex selectionIndex = m_pTreeView->model()->index((int)scheduledTransactionIndex, 0);
	m_pTreeView->selectionModel()->select(selectionIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	m_pTreeView->scrollTo(selectionIndex);
}

QModelIndex ScheduledTransactionsViewWidget::getSingleSelectedIndex() const
{
	QModelIndex index;
	
	QModelIndexList selectedIndices = m_pTreeView->selectionModel()->selectedIndexes();
	if (!selectedIndices.empty())
	{
		index = selectedIndices[0];
	}
	
	return index;
}

void ScheduledTransactionsViewWidget::updateItemButtonsFromSelection()
{
	unsigned int buttonsEnabled = ItemControlButtonsWidget::eBtnAdd;
	
	if (m_scheduledTransactionIndex != -1)
	{
		buttonsEnabled |= ItemControlButtonsWidget::eBtnDelete;
	}
	
	m_pItemControlButtons->setButtonsEnabled(buttonsEnabled);
}
