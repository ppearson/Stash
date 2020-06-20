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

#ifndef DOCUMENT_INDEX_DATAMODEL_H
#define DOCUMENT_INDEX_DATAMODEL_H

#include <QAbstractItemModel>
#include <QVector>

#include "view_common.h"

class Document;
class DocumentIndexModelItem;

class DocumentIndexDataModel : public QAbstractItemModel
{
public:
	DocumentIndexDataModel(Document& document, QObject* parent = 0);
	virtual ~DocumentIndexDataModel();
	
	virtual QVariant data(const QModelIndex& index, int role) const;

//	virtual bool hasChildren(const QModelIndex& parent) const;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	DocumentIndexModelItem* getItem(const QModelIndex& index) const;

	void rebuildModelFromDocument();
	void clear();
	
	
protected:
	Document&				m_document;
	
	bool					m_upToDate;

	DocumentIndexModelItem*	m_pRootItem;
};

class DocumentIndexModelItem
{
public:
	
	DocumentIndexModelItem(const QVariant& data, DocumentIndexModelItem* parent, DocumentIndexType type);
	~DocumentIndexModelItem();
	
	DocumentIndexModelItem* child(int number) const;
	int childCount() const;

	const QVariant& mainData() const;
	const QVariant& secondaryData() const;
	
	DocumentIndexType getType() const
	{
		return m_type;
	}

	DocumentIndexModelItem* parent();

	int childNumber() const;

	void setMainData(const QVariant& value);
	void setSecondaryData(const QVariant& value);

	void addChild(DocumentIndexModelItem* pChild);


protected:
	QVector<DocumentIndexModelItem*>	m_childItems;
	DocumentIndexModelItem*				m_parentItem;
	
	// generally the main name
	QVariant						m_itemMainData;
	// balance for things like accounts
	QVariant						m_itemSecondaryData;

	DocumentIndexType				m_type;
};

#endif // DOCUMENT_INDEX_DATAMODEL_H
