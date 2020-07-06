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

#include "document_index_data_model.h"

DocumentIndexView::DocumentIndexView(Document& document, QWidget* parent) : QWidget(parent),
	m_document(document),
	m_pTreeView(nullptr), m_pModel(nullptr),
	m_selectedIndexSubIndex(eDocIndex_None)
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

	m_pModel = new DocumentIndexDataModel(m_document, this);

	m_pTreeView->setModel(m_pModel);

	m_pModel->rebuildModelFromDocument();

	layout->addWidget(m_pTreeView);

	connect(m_pTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
			SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
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
	
	if (m_selectIndexType != eDocIndex_None)
	{
		selectItem(m_selectIndexType, m_selectedIndexSubIndex, false);
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
		if (!sendSelectionChangedEvent)
		{
			m_pTreeView->selectionModel()->blockSignals(false);
		}
	}
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
		m_selectIndexType = itemType;
		m_selectedIndexSubIndex = childIndex;
		
		emit indexSelectionHasChanged(itemType, childIndex);
	}
}
