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

#include "transactions_view_data_model.h"

#include <QLocale>

#include "../../core/account.h"

static const int kRemainderSplitTransactionIndex = -2;

TransactionsViewDataModel::TransactionsViewDataModel(QObject* parent) : QAbstractItemModel(parent), 
	m_pAccount(nullptr),
	m_pRootItem(nullptr),
	m_transactionsViewType(eTransViewShowRecent),
	m_valid(false)
{

}

TransactionsViewDataModel::~TransactionsViewDataModel()
{
	delete m_pRootItem;
}

void TransactionsViewDataModel::setAccount(Account* pAccount)
{
	m_pAccount = pAccount;
	
	m_valid = m_pAccount != nullptr;
}

void TransactionsViewDataModel::setTransactionsViewDurationType(TransactionsViewDurationType viewType)
{
	if (viewType == m_transactionsViewType)
		return;
	
	m_transactionsViewType = viewType;
	
	rebuildModelFromAccount();
}

QVariant TransactionsViewDataModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		TransactionsModelItem* item = getItem(index);
		if (item)
		{
			if (index.column() == 0)
			{
				return item->getDate();
			}
			else if (index.column() == 1)
			{
				return item->getPayee();
			}
			else if (index.column() == 2)
			{
				return item->getCategory();
			}
			else if (index.column() == 3)
			{
				return item->getDescription();
			}
			else if (index.column() == 4)
			{
				return item->getAmount();
			}
			else if (index.column() == 5)
			{
				return item->getBalance();
			}
		}
	}
	else if (role == Qt::DecorationRole)
	{
		TransactionsModelItem* item = getItem(index);
		if (item)
		{
			
		}
	}
	else if (role == Qt::TextColorRole)
	{
		TransactionsModelItem* item = getItem(index);
		if (item)
		{
			if (item->getSplitTransactionIndex() == kRemainderSplitTransactionIndex)
			{
				return QVariant(Qt::darkGray);
			}
		}
	}
	else if (role == Qt::BackgroundColorRole)
	{

	}
	else if (role == Qt::TextAlignmentRole)
	{
		return (index.column() == 4 || index.column() == 5) ? Qt::AlignRight : Qt::AlignLeft;
	}

	return QVariant();
}

QVariant TransactionsViewDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();
	
	if (orientation == Qt::Horizontal)
	{
		if (section == 0)
			return QVariant("Date");
		else if (section == 1)
			return QVariant("Payee");
		else if (section == 2)
			return QVariant("Category");
		else if (section == 3)
			return QVariant("Description");
		else if (section == 4)
			return QVariant("Amount");
		else if (section == 5)
			return QVariant("Balance");
	}
	
	return QVariant();
}

QModelIndex TransactionsViewDataModel::index(int row, int column, const QModelIndex& parent) const
{		
	// ?
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	TransactionsModelItem* parentItem = getItem(parent);

	TransactionsModelItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex TransactionsViewDataModel::parent(const QModelIndex& index) const
{
	if (!m_valid || !index.isValid())
		return QModelIndex();

	TransactionsModelItem* childItem = getItem(index);
	TransactionsModelItem* parentItem = childItem->parent();

	if (!parentItem || parentItem == m_pRootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TransactionsViewDataModel::columnCount(const QModelIndex& parent) const
{
	return 6;
}

int TransactionsViewDataModel::rowCount(const QModelIndex& parent) const
{
	if (!m_valid)
		return 0;
	
	TransactionsModelItem* parentItem = getItem(parent);

	return parentItem->childCount();
}

TransactionsModelItem* TransactionsViewDataModel::getItem(const QModelIndex& index) const
{
	if (index.isValid())
	{
		TransactionsModelItem* item = static_cast<TransactionsModelItem*>(index.internalPointer());
		if (item)
			return item;
	}

	return m_pRootItem;
}

void TransactionsViewDataModel::rebuildModelFromAccount()
{
	if (!m_pAccount)
	{
		// just reset and then return
		
		beginResetModel();
		if (m_pRootItem)
			delete m_pRootItem;
	
		m_pRootItem = new TransactionsModelItem(nullptr);
		endResetModel();
		
		return;
	}
	
	QLocale locale;
	
	beginResetModel();
	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new TransactionsModelItem(nullptr);
	
	fixed balance = 0.0;
	
	std::vector<Transaction>::const_iterator it = m_pAccount->begin();
	std::vector<Transaction>::const_iterator itEnd = m_pAccount->end();
	
	unsigned int transactionIndex = 0;
	
	// calculate the starting transaction if necessary
	if (m_transactionsViewType != eTransViewShowAll)
	{
		Date dateCompare;
		dateCompare.Now();
		
		if (m_transactionsViewType == eTransViewShowRecent)
		{
			// TODO: hook this up to Settings/Options...
			int nRecentDuration = 30;
			
			if (nRecentDuration <= 0)
				nRecentDuration = 30;
			
			dateCompare.DecrementDays(nRecentDuration);
		}
		else // all this year
		{
			dateCompare.setDay(1);
			dateCompare.setMonth(1);
		}
		
		std::vector<Transaction>::const_iterator itTemp = m_pAccount->begin();
		std::vector<Transaction>::const_iterator itTempEnd = m_pAccount->end();
		
		for (; itTemp != itTempEnd; ++itTemp)
		{
			if ((*itTemp).getDate() >= dateCompare)
				break;
			
			balance += (*itTemp).getAmount();
			transactionIndex++;
		}
		
		it = itTemp;
	}
	
	for (; it != itEnd; ++it)
	{
		const Transaction& transaction = *it;
		const double transactionAmount = transaction.getAmount().ToDouble();
		
		TransactionsModelItem* pNewItem = new TransactionsModelItem(m_pRootItem);
		
		pNewItem->extractDetails(transaction, transactionIndex);
		
		// TODO: QLocale::toCurrencyString() is pretty useless really, it doesn't put the
		//       currency in the right place for negative values, and doesn't apply grouping
		//       (thousands seperator).
		pNewItem->setAmount(locale.toCurrencyString(transactionAmount));
		
		if (transaction.isCleared())
		{
			balance += transactionAmount;
		}
		
		pNewItem->setBalance(locale.toCurrencyString(balance.ToDouble()));
		
		m_pRootItem->addChild(pNewItem);
		
		transactionIndex++;
	}

	endResetModel();
	
	// resetting the model doesn't reset the scroll position, so that needs to be done externally (in TransactionsViewWidget)	
}

void TransactionsViewDataModel::clear()
{
	beginResetModel();

	if (m_pRootItem)
		delete m_pRootItem;

	m_pRootItem = new TransactionsModelItem(nullptr);

	m_valid = false;

	endResetModel();
}

//

TransactionsModelItem::TransactionsModelItem(TransactionsModelItem* parent) : 
	m_parentItem(parent),
	m_transactionIndex(-1),
	m_splitTransactionIndex(-1)
{
	
}

TransactionsModelItem::~TransactionsModelItem()
{
	qDeleteAll(m_childItems);
}

void TransactionsModelItem::extractDetails(const Transaction& transaction, unsigned int transactionIndex)
{
	m_date = transaction.getDate().FormattedDate(Date::UK).c_str();
	m_payee = transaction.getPayee().c_str();
	m_category = transaction.getCategory().c_str();
	m_description = transaction.getDescription().c_str();
	
	m_transactionIndex = transactionIndex;
	
	QLocale locale;
	
	if (transaction.isSplit())
	{
		unsigned int numSplits = transaction.getSplitCount();
		
		fixed remainingSplitAmount = transaction.getAmount();
		
		for (unsigned int i = 0; i < numSplits; i++)
		{
			const SplitTransaction& splitTrans = transaction.getSplit(i);
			
			// because we're part of our own class, we can access member variables directly...
			TransactionsModelItem* pNewSplitItem = new TransactionsModelItem(this);
			
			pNewSplitItem->m_payee = splitTrans.getPayee().c_str();
			
			pNewSplitItem->m_category = splitTrans.getCategory().c_str();
			pNewSplitItem->m_description = splitTrans.getDescription().c_str();
			
			// TODO: QLocale is pretty useless...
			pNewSplitItem->setAmount(locale.toCurrencyString(splitTrans.getAmount().ToDouble()));
			
			pNewSplitItem->m_transactionIndex = transactionIndex;
			pNewSplitItem->m_splitTransactionIndex = (int)i;
			
			addChild(pNewSplitItem);
			
			remainingSplitAmount -= splitTrans.getAmount();
		}
		
		if (!remainingSplitAmount.IsZero())
		{
			TransactionsModelItem* pNewRemainderSplitItem = new TransactionsModelItem(this);
			
			pNewRemainderSplitItem->setAmount(locale.toCurrencyString(remainingSplitAmount.ToDouble()));
			
			pNewRemainderSplitItem->m_payee = "Split Value";
			pNewRemainderSplitItem->m_description = "Split Value";
			
			pNewRemainderSplitItem->m_transactionIndex = transactionIndex;
			pNewRemainderSplitItem->m_splitTransactionIndex = kRemainderSplitTransactionIndex; // denote it's a split remainder
			
			addChild(pNewRemainderSplitItem);
		}
	}
}

void TransactionsModelItem::setAmount(const QString& value)
{
	m_amount = value;
}

void TransactionsModelItem::setBalance(const QString& value)
{
	m_balance = value;
}

TransactionsModelItem* TransactionsModelItem::child(int number) const
{
	if (number >= m_childItems.count())
		return nullptr;

	return m_childItems.value(number);
}

int TransactionsModelItem::childCount() const
{
	return m_childItems.count();
}

TransactionsModelItem* TransactionsModelItem::parent()
{
	return m_parentItem;
}

int TransactionsModelItem::childNumber() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<TransactionsModelItem*>(this));

	return 0;
}

void TransactionsModelItem::addChild(TransactionsModelItem* pChild)
{
	m_childItems.push_back(pChild);
}
