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

#ifndef DOCUMENT_INDEX_VIEW_H
#define DOCUMENT_INDEX_VIEW_H

#include <QWidget>

#include "view_common.h"

class QTreeView;
class QItemSelection;
class QAction;

class StashWindow;

class Document;
class DocumentIndexDataModel;

class DocumentIndexView : public QWidget
{
	Q_OBJECT
public:
	DocumentIndexView(Document& document, QWidget* parent, StashWindow* stashWindow);
	virtual ~DocumentIndexView();
	
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;

	void rebuildFromDocument();
	
	void selectItem(DocumentIndexType type, unsigned int index, bool sendSelectionChangedEvent = true);
	
signals:
	void indexSelectionHasChanged(DocumentIndexType type, int index);

	void documentChangedFromIndex();
	
	void deselectAnyAccount(); // for selecting a non-existant account after deleting the last account

public slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	
	void customContextMenuRequested(const QPoint& pos);
	
	void menuAccountDetails();
	void menuAccountDelete();
	

protected:
	Document&				m_document;
	
	QTreeView*				m_pTreeView;
	DocumentIndexDataModel*	m_pModel;
	
	QAction*				m_pAccountDetails;
	QAction*				m_pAccountDelete;
	
	
	// current / last selection state
	DocumentIndexType		m_selectedIndexType;
	unsigned int			m_selectedIndexSubIndex;
};

#endif // DOCUMENT_INDEX_VIEW_H
