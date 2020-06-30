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

#include "document_index_data_model.h"

#include <QColor>

#include "../../core/account.h"
#include "../../core/document.h"

DocumentIndexDataModel::DocumentIndexDataModel(const Document& document, QObject* parent) : QAbstractItemModel(parent), 
		m_document(document),
		m_upToDate(false), m_pRootItem(nullptr)
{
	
}

DocumentIndexDataModel::~DocumentIndexDataModel()
{
	delete m_pRootItem;
}

QVariant DocumentIndexDataModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	DocumentIndexModelItem* item = getItem(index);
	
	if (role == Qt::DisplayRole)
	{
		if (index.column() == 1)
		{
			return item->secondaryData();
		}
		else
		{
			return item->mainData();
		}
	}
	else if (role == Qt::DecorationRole)
	{
		
	}
	else if (role == Qt::TextColorRole)
	{
		if (index.column() == 1)
		{
			return QColor(Qt::darkGray);
		}
		else
		{
			if (item->getType() == eDocIndex_Heading)
			{
				return QColor(Qt::darkGray);
			}
			else
			{
				return QColor(Qt::black);
			}
		}
	}
	else if (role == Qt::TextAlignmentRole)
	{
		return (index.column() == 1) ? Qt::AlignRight : Qt::AlignLeft;
	}

	return QVariant();
}

QModelIndex DocumentIndexDataModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	DocumentIndexModelItem* parentItem = getItem(parent);

	DocumentIndexModelItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex DocumentIndexDataModel::parent(const QModelIndex& index) const
{
	if (!m_upToDate || !index.isValid())
		return QModelIndex();

	DocumentIndexModelItem* childItem = getItem(index);
	DocumentIndexModelItem* parentItem = childItem->parent();

	if (!parentItem || parentItem == m_pRootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int DocumentIndexDataModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

int DocumentIndexDataModel::rowCount(const QModelIndex& parent) const
{
	DocumentIndexModelItem* parentItem = getItem(parent);

	return parentItem->childCount();
}

DocumentIndexModelItem* DocumentIndexDataModel::getItem(const QModelIndex& index) const
{
	if (index.isValid())
	{
		DocumentIndexModelItem* item = static_cast<DocumentIndexModelItem*>(index.internalPointer());
		if (item)
			return item;
	}

	return m_pRootItem;
}

void DocumentIndexDataModel::rebuildModelFromDocument()
{
	beginResetModel();
	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new DocumentIndexModelItem(QString("Document"), nullptr, eDocIndex_Heading);

	DocumentIndexModelItem* pAccounts = new DocumentIndexModelItem(QString("Accounts"), m_pRootItem, eDocIndex_Heading);
	m_pRootItem->addChild(pAccounts);
	
	char szTemp[16];
	
	const std::vector<Account>& aAccounts = m_document.getAccounts();
	for (const Account& account : aAccounts)
	{
		DocumentIndexModelItem* pAccN = new DocumentIndexModelItem(QString(account.getName().c_str()), pAccounts, eDocIndex_Account);
		
		fixed balance = account.getBalance(true, -1);
		double dBalance = balance.ToDouble();
		
		// TODO: do this properly...
		sprintf(szTemp, "$%0.2f", dBalance);
		
		pAccN->setSecondaryData(szTemp);

		pAccounts->addChild(pAccN);
	}

	// Manage
	
	DocumentIndexModelItem* pManage = new DocumentIndexModelItem(QString("Manage"), m_pRootItem, eDocIndex_Heading);
	m_pRootItem->addChild(pManage);
	
	pManage->addChild(new DocumentIndexModelItem(QString("Payees"), pManage, eDocIndex_Manage));
	pManage->addChild(new DocumentIndexModelItem(QString("Categories"), pManage, eDocIndex_Manage));
	pManage->addChild(new DocumentIndexModelItem(QString("Scheduled"), pManage, eDocIndex_Manage));
	
	
	// Graphs
	
	DocumentIndexModelItem* pGraphs = new DocumentIndexModelItem(QString("Graphs"), m_pRootItem, eDocIndex_Heading);
	m_pRootItem->addChild(pGraphs);

	m_upToDate = true;
	endResetModel();
}

void DocumentIndexDataModel::clear()
{
	beginResetModel();

	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new DocumentIndexModelItem(QString("Document"), nullptr, eDocIndex_Heading);

	DocumentIndexModelItem* pAccounts = new DocumentIndexModelItem(QString("Accounts"), m_pRootItem, eDocIndex_Heading);
	m_pRootItem->addChild(pAccounts);

	m_upToDate = false;

	endResetModel();
}

//

DocumentIndexModelItem::DocumentIndexModelItem(const QVariant& data, DocumentIndexModelItem* parent, DocumentIndexType type) : 
    m_parentItem(parent), m_itemMainData(data), m_type(type)
{
	
}

DocumentIndexModelItem::~DocumentIndexModelItem()
{
	qDeleteAll(m_childItems);
}

DocumentIndexModelItem* DocumentIndexModelItem::child(int number) const
{
	if (number >= m_childItems.count())
		return nullptr;

	return m_childItems.value(number);
}

int DocumentIndexModelItem::childCount() const
{
	return m_childItems.count();
}

const QVariant& DocumentIndexModelItem::mainData() const
{
	return m_itemMainData;
}

const QVariant& DocumentIndexModelItem::secondaryData() const
{
	return m_itemSecondaryData;
}

DocumentIndexModelItem* DocumentIndexModelItem::parent()
{
	return m_parentItem;
}

int DocumentIndexModelItem::childNumber() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<DocumentIndexModelItem*>(this));

	return 0;
}

void DocumentIndexModelItem::setMainData(const QVariant& value)
{
	m_itemMainData = value;
}

void DocumentIndexModelItem::setSecondaryData(const QVariant& value)
{
	m_itemSecondaryData = value;
}

void DocumentIndexModelItem::addChild(DocumentIndexModelItem* pChild)
{
	m_childItems.push_back(pChild);
}

