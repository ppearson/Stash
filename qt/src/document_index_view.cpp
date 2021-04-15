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

#include "document_index_view.h"

#include <QTreeView>
#include <QLayout>
#include <QAction>
#include <QMouseEvent>
#include <QMenu>
#include <QMessageBox>

#include "../../core/document.h"

#include "document_index_data_model.h"

#include "dialogs/account_details_dialog.h"

DocumentIndexView::DocumentIndexView(Document& document, QWidget* parent, StashWindow* stashWindow) : QWidget(parent),
	m_document(document),
	m_pTreeView(nullptr),
	m_pModel(nullptr),
	m_pAccountDetails(nullptr),
	m_selectedIndexType(eDocIndex_None),
	m_selectedIndexSubIndex(-1)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);

	setLayout(layout);

	m_pTreeView = new QTreeView(this);
	m_pTreeView->setHeaderHidden(true);
//	m_pTreeView->setFrameStyle(QFrame::NoFrame);
	m_pTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pTreeView->setAttribute(Qt::WA_MacShowFocusRect, false); // hide the OS X blue focus rect

	m_pModel = new DocumentIndexDataModel(m_document, this, stashWindow);

	m_pTreeView->setModel(m_pModel);

	m_pModel->rebuildModelFromDocument();

	layout->addWidget(m_pTreeView);

	connect(m_pTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
			SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	
	m_pTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(customContextMenuRequested(const QPoint&)));
	
	m_pAccountDetails = new QAction(this);
	m_pAccountDetails->setText("Account details...");
	
	m_pAccountDelete = new QAction(this);
	m_pAccountDelete->setText("Delete Account...");
	
	m_pGraphDelete = new QAction(this);
	m_pGraphDelete->setText("Delete Graph...");
	
	connect(m_pAccountDetails, SIGNAL(triggered()), this, SLOT(menuAccountDetails()));
	connect(m_pAccountDelete, SIGNAL(triggered()), this, SLOT(menuAccountDelete()));
	
	connect(m_pGraphDelete, SIGNAL(triggered()), this, SLOT(menuGraphDelete()));
}

DocumentIndexView::~DocumentIndexView()
{
	// TODO: are these even needed if we're parenting the QActions?
	if (m_pAccountDetails)
	{
		delete m_pAccountDetails;
		m_pAccountDetails = nullptr;
	}
	
	if (m_pAccountDelete)
	{
		delete m_pAccountDelete;
		m_pAccountDelete = nullptr;
	}
	
	if (m_pGraphDelete)
	{
		delete m_pGraphDelete;
		m_pGraphDelete = nullptr;
	}
}

QSize DocumentIndexView::minimumSizeHint() const
{
	return QSize(10, 10);
}

QSize DocumentIndexView::sizeHint() const
{
	return QSize(100, 100);
}

void DocumentIndexView::rebuildFromDocument()
{
	m_pTreeView->blockSignals(true);
	m_pTreeView->selectionModel()->blockSignals(true);

	m_pTreeView->selectionModel()->clear();

	m_pModel->clear();
	m_pModel->rebuildModelFromDocument();

	m_pTreeView->reset();
	
	m_pTreeView->expandAll();
	
	if (m_selectedIndexType != eDocIndex_None)
	{
		selectItem(m_selectedIndexType, m_selectedIndexSubIndex, false);
	}
	
	m_pTreeView->selectionModel()->blockSignals(false);
	m_pTreeView->blockSignals(false);
	
	m_pTreeView->resizeColumnToContents(0);
	m_pTreeView->resizeColumnToContents(1);
}

void DocumentIndexView::selectItem(DocumentIndexType type, unsigned int index, bool sendSelectionChangedEvent)
{
	QModelIndex itemToSelect;
	if (type == eDocIndex_Account)
	{
		itemToSelect = m_pModel->index(0, 0); // first level
		itemToSelect = itemToSelect.child(index, 0);
	}
	
	if (itemToSelect.isValid())
	{
		// because calling select() to select an already selected item apparently doesn't trigger the selectionChanged()
		// event, we need to reset selection first, then re-apply it (playing with the selection flags doesn't seem to help)
		
		if (!sendSelectionChangedEvent)
		{
			m_pTreeView->selectionModel()->blockSignals(true);
		}
		else
		{
			// reset selection first
			m_pTreeView->selectionModel()->clearSelection();
		}
		m_pTreeView->selectionModel()->select(itemToSelect, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		// this isn't strictly-speaking needed, however, QTreeView has differing selected/current state, and the current state
		// can in some situations show highlight boxes on the first item, which is confusing, so...
		m_pTreeView->setCurrentIndex(itemToSelect);
		if (!sendSelectionChangedEvent)
		{
			m_pTreeView->selectionModel()->blockSignals(false);
		}
	}
}

int DocumentIndexView::getSelectedAccountIndex() const
{
	if (m_selectedIndexType != eDocIndex_Account)
		return -1;
	
	return (int)m_selectedIndexSubIndex;
}

void DocumentIndexView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QList<QModelIndex> indexes = selected.indexes();
	// just select the first for now

	if (indexes.isEmpty())
		return;

	QModelIndex& index = indexes[0];
	if (!index.isValid())
		return;
	
	DocumentIndexModelItem* item = static_cast<DocumentIndexModelItem*>(index.internalPointer());
	if (item)
	{
		DocumentIndexType itemType = item->getType();
		
		int childIndex = item->childNumber();
		
		// keep track of last selected
		m_selectedIndexType = itemType;
		m_selectedIndexSubIndex = childIndex;
		
		emit indexSelectionHasChanged(itemType, childIndex);
	}
}

void DocumentIndexView::customContextMenuRequested(const QPoint& pos)
{
	if (m_selectedIndexType == eDocIndex_Account)
	{
		QMenu menu(this);
	
		menu.addAction(m_pAccountDetails);
		menu.addSeparator();
		menu.addAction(m_pAccountDelete);
		menu.exec(mapToGlobal(pos));
	}
	else if (m_selectedIndexType == eDocIndex_Graph)
	{
		QMenu menu(this);
	
		menu.addAction(m_pGraphDelete);
		menu.exec(mapToGlobal(pos));
	}
}

void DocumentIndexView::menuAccountDetails()
{
	Account& account = m_document.getAccount(m_selectedIndexSubIndex);
	
	AccountDetailsDialog accountDetailsDlg(this, false);
	accountDetailsDlg.setFromAccount(account);
	
	if (accountDetailsDlg.exec() == QDialog::Accepted)
	{
		account.setName(accountDetailsDlg.getAccountName());
		account.setInstitution(accountDetailsDlg.getAccountInstitution());
		account.setNumber(accountDetailsDlg.getAccountNumber());
		account.setNote(accountDetailsDlg.getAccountNote());
		account.setType(accountDetailsDlg.getAccountType());
		
		// TODO: technically, we only need to do this if the Account Name changed, but...
		rebuildFromDocument();
		
		emit documentChangedFromIndex();
	}
}

void DocumentIndexView::menuAccountDelete()
{
	// shouldn't need this, but...
	if (m_selectedIndexSubIndex == -1u || m_selectedIndexSubIndex >= m_document.getAccountCount())
		return;
	
	const Account& account = m_document.getAccount(m_selectedIndexSubIndex);
	
	int ret = QMessageBox::question(this, tr("Stash"), tr("Are you sure you want to delete the account called '") + QString(account.getName().c_str()) + "'?",
					QMessageBox::No | QMessageBox::Default, QMessageBox::Yes);

	if (ret == QMessageBox::No)
		return;
	
	// otherwise, delete the account.
	
	m_document.deleteAccount((int)m_selectedIndexSubIndex);
	
	rebuildFromDocument();
	
	// force a different account to show...
	
	if (m_document.getAccountCount() == 0)
	{
		// don't really like this, but we need to do *something* in this situation, so for the moment...
		emit deselectAnyAccount();
	}
	else
	{
		// select an existing account
		unsigned int nextAccountIndex = (m_selectedIndexSubIndex >= m_document.getAccountCount()) ?
										m_document.getAccountCount() - 1 : m_selectedIndexSubIndex;
		
		selectItem(eDocIndex_Account, nextAccountIndex, true);
	}
	
	emit documentChangedFromIndex();
}

void DocumentIndexView::menuGraphDelete()
{
	// shouldn't need this, but...
	if (m_selectedIndexSubIndex == -1u || m_selectedIndexSubIndex >= m_document.getGraphCount())
		return;
	
	const Graph& graph = m_document.getGraph(m_selectedIndexSubIndex);
	
	int ret = QMessageBox::question(this, tr("Stash"), tr("Are you sure you want to delete the graph called '") + QString(graph.getName().c_str()) + "'?",
					QMessageBox::No | QMessageBox::Default, QMessageBox::Yes);

	if (ret == QMessageBox::No)
		return;
	
	// otherwise, delete the account.
	
	m_document.deleteGraph((int)m_selectedIndexSubIndex);
	
	rebuildFromDocument();
}
