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

#include "scheduled_transactions_view_data_model.h"

#include <QLocale>

#include "../../core/document.h"
#include "../../core/scheduled_transaction.h"

#include "ui_currency_handler.h"
#include "ui_date_handler.h"

#include "stash_window.h"

ScheduledTransactionsViewDataModel::ScheduledTransactionsViewDataModel(StashWindow* pMainWindow, Document& document, QObject* parent) :
	QAbstractItemModel(parent),
	m_pMainWindow(pMainWindow),
	m_document(document),
	m_pRootItem(nullptr)
{
	rebuildModelFromDocument();
}

ScheduledTransactionsViewDataModel::~ScheduledTransactionsViewDataModel()
{
	delete m_pRootItem;
}

QVariant ScheduledTransactionsViewDataModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		ScheduledTransactionsModelItem* item = getItem(index);
		if (item)
		{
			if (index.column() == 1)
			{
				return item->getPayee();
			}
			else if (index.column() == 2)
			{
				return item->getCategory();
			}
			else if (index.column() == 3)
			{
				return item->getAmount();
			}
			else if (index.column() == 4)
			{
				return item->getFrequency();
			}
			else if (index.column() == 5)
			{
				return item->getNextDate();
			}
			else if (index.column() == 6)
			{
				return item->getAccount();
			}
		}
	}
	else if (role == Qt::DecorationRole)
	{
		
	}
	else if (role == Qt::CheckStateRole)
	{
		if (index.column() == 0)
		{
			ScheduledTransactionsModelItem* item = getItem(index);
			if (item)
			{
				return (item->getEnabled() == true) ? Qt::Checked : Qt::Unchecked;
			}
		}
	}
	else if (role == Qt::SizeHintRole)
	{
		if (index.column() == 0)
		{
			// force a smaller icon size than default, so that there's less padding
			// around the checked icons, and so therefore the row hights are the same as
			// without any checkbox.
			return QSize(16, 16);
		}
	}
	else if (role == Qt::TextAlignmentRole)
	{
		return (index.column() == 3) ? Qt::AlignRight : Qt::AlignLeft;
	}

	return QVariant();
}

Qt::ItemFlags ScheduledTransactionsViewDataModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return 0;
	
	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	
	if (index.column() == 0)
		flags |= Qt::ItemIsUserCheckable;
	
	return flags;
}

QVariant ScheduledTransactionsViewDataModel::headerData(int section, Qt::Orientation orientation,
														int role) const
{
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();
	
	if (orientation == Qt::Horizontal)
	{
		if (section == 0)
			return QVariant("Enabled");
		else if (section == 1)
			return QVariant("Payee");
		else if (section == 2)
			return QVariant("Category");
		else if (section == 3)
			return QVariant("Amount");
		else if (section == 4)
			return QVariant("Frequency");
		else if (section == 5)
			return QVariant("Next Date");
		else if (section == 6)
			return QVariant("Account");
	}
	
	return QVariant();
}

bool ScheduledTransactionsViewDataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;
	
	if (index.column() == 0 && role == Qt::CheckStateRole)
	{
		ScheduledTransactionsModelItem* item = getItem(index);
		item->setEnabled(value == Qt::Checked);
		
		// update the data in the actual scheduled transaction
		ScheduledTransaction& scheduledTransaction = m_document.getScheduledTransaction(item->getScheduledTransactionIndex());
		scheduledTransaction.setEnabled(item->getEnabled().toBool());
		
		// notify main window that the document has been updated
		m_pMainWindow->setWindowModifiedAndRebuildIndex(false); // we don't need to rebuild the index.
		
		return true;
	}
	
	return false;
}

QModelIndex ScheduledTransactionsViewDataModel::index(int row, int column, const QModelIndex& parent) const
{
	// ?
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	ScheduledTransactionsModelItem* parentItem = getItem(parent);

	ScheduledTransactionsModelItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex ScheduledTransactionsViewDataModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	ScheduledTransactionsModelItem* childItem = getItem(index);
	ScheduledTransactionsModelItem* parentItem = childItem->parent();

	if (!parentItem || parentItem == m_pRootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int ScheduledTransactionsViewDataModel::columnCount(const QModelIndex& parent) const
{
	return 7;
}

int ScheduledTransactionsViewDataModel::rowCount(const QModelIndex& parent) const
{
	ScheduledTransactionsModelItem* parentItem = getItem(parent);

	return parentItem->childCount();
}

ScheduledTransactionsModelItem* ScheduledTransactionsViewDataModel::getItem(const QModelIndex& index) const
{
	if (index.isValid())
	{
		ScheduledTransactionsModelItem* item = static_cast<ScheduledTransactionsModelItem*>(index.internalPointer());
		if (item)
			return item;
	}

	return m_pRootItem;
}

void ScheduledTransactionsViewDataModel::rebuildModelFromDocument()
{
	beginResetModel();
	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new ScheduledTransactionsModelItem(nullptr);
	
	std::vector<ScheduledTransaction>::const_iterator schedTransIt = m_document.SchedTransBegin();
	
	int schedTransIndex = 0;
	
	UICurrencyHandler* pCurrencyHandler = m_pMainWindow->getCurrencyHandler();
	UIDateHandler* pDateHandler = m_pMainWindow->getDateHandler();
	
	const Document& document = m_pMainWindow->getDocumentController().getDocument();
	
	for (; schedTransIt != m_document.SchedTransEnd(); ++schedTransIt, schedTransIndex++)
	{
		const ScheduledTransaction& schedTrans = *schedTransIt;
		
		ScheduledTransactionsModelItem* newModelItem = new ScheduledTransactionsModelItem(m_pRootItem);
			
		newModelItem->m_enabled = schedTrans.isEnabled();
		newModelItem->m_payee = schedTrans.getPayee().c_str();
		newModelItem->m_category = schedTrans.getCategory().c_str();
		
		newModelItem->m_amount = pCurrencyHandler->formatCurrencyAmount(schedTrans.getAmount());
		
		switch (schedTrans.getFrequency())
		{
		case 0:
			newModelItem->m_frequency = "Weekly";
			break;
		case 1:
			newModelItem->m_frequency = "Two Weeks";
			break;
		case 2:
			newModelItem->m_frequency = "Four Weeks";
			break;
		case 3:
			newModelItem->m_frequency = "Monthly";
			break;
		case 4:
			newModelItem->m_frequency = "Two Months";
			break;
		case 5:
			newModelItem->m_frequency = "Quarterly";
			break;
		case 6:
			newModelItem->m_frequency = "Annually";
			break;
		}
		
		newModelItem->m_nextDate = pDateHandler->formatDate(schedTrans.getNextDate());
		
		// it's not great having to do this...
		unsigned int accountIndex = schedTrans.getAccount();
		if (accountIndex != -1u && accountIndex < document.getAccountCount())
		{
			const Account& acc = document.getAccount(accountIndex);
			newModelItem->m_account = acc.getName().c_str();
		}
		
		newModelItem->m_scheduledTransactionIndex = schedTransIndex;
	
		m_pRootItem->addChild(newModelItem);
	}
	
	endResetModel();
}

void ScheduledTransactionsViewDataModel::clear()
{
	beginResetModel();

	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new ScheduledTransactionsModelItem(nullptr);

	endResetModel();
}

//

ScheduledTransactionsModelItem::ScheduledTransactionsModelItem(ScheduledTransactionsModelItem* parent) :
	m_parentItem(parent),
	m_scheduledTransactionIndex(-1)
{
	
}

ScheduledTransactionsModelItem::~ScheduledTransactionsModelItem()
{
	qDeleteAll(m_childItems);
}

ScheduledTransactionsModelItem* ScheduledTransactionsModelItem::child(int number) const
{
	if (number >= m_childItems.count())
		return nullptr;

	return m_childItems.value(number);
}

int ScheduledTransactionsModelItem::childCount() const
{
	return m_childItems.count();
}

ScheduledTransactionsModelItem* ScheduledTransactionsModelItem::parent()
{
	return m_parentItem;
}

int ScheduledTransactionsModelItem::childNumber() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<ScheduledTransactionsModelItem*>(this));

	return 0;
}

void ScheduledTransactionsModelItem::addChild(ScheduledTransactionsModelItem* pChild)
{
	m_childItems.push_back(pChild);
}
