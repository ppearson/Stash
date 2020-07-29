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

#ifndef SCHEDULED_TRANSACTIONS_VIEW_DATA_MODEL_H
#define SCHEDULED_TRANSACTIONS_VIEW_DATA_MODEL_H

#include <QAbstractItemModel>
#include <QVector>

class Document;
class ScheduledTransaction;

class StashWindow;

class ScheduledTransactionsModelItem;

class ScheduledTransactionsViewDataModel : public QAbstractItemModel
{
public:
	ScheduledTransactionsViewDataModel(StashWindow* pMainWindow, Document& document, QObject* parent);
	virtual ~ScheduledTransactionsViewDataModel();
	
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	
	virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
	
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

//	virtual bool hasChildren(const QModelIndex& parent) const;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	ScheduledTransactionsModelItem* getItem(const QModelIndex& index) const;

	void rebuildModelFromDocument();
	void clear();
	
protected:
	StashWindow*					m_pMainWindow;
	Document&						m_document;
	
	ScheduledTransactionsModelItem*	m_pRootItem;
};

class ScheduledTransactionsModelItem
{
public:
	ScheduledTransactionsModelItem(ScheduledTransactionsModelItem* parent = nullptr);
	~ScheduledTransactionsModelItem();
	
	friend class ScheduledTransactionsViewDataModel;
	
	ScheduledTransactionsModelItem* child(int number) const;
	int childCount() const;

	ScheduledTransactionsModelItem* parent();

	int childNumber() const;

	void addChild(ScheduledTransactionsModelItem* pChild);
	
	// getters for data
	
	QVariant getEnabled() const
	{
		return m_enabled;
	}
	
	void setEnabled(QVariant enabled)
	{
		m_enabled = enabled;
	}
	
	QVariant getPayee() const
	{
		return m_payee;
	}
	
	QVariant getCategory() const
	{
		return m_category;
	}
	
	QVariant getAmount() const
	{
		return m_amount;
	}
	
	QVariant getFrequency() const
	{
		return m_frequency;
	}
	
	QVariant getNextDate() const
	{
		return m_nextDate;
	}
	
	QVariant getAccount() const
	{
		return m_account;
	}
	
	unsigned int getScheduledTransactionIndex() const
	{
		return m_scheduledTransactionIndex;
	}
	

protected:
	QVector<ScheduledTransactionsModelItem*>	m_childItems;
	ScheduledTransactionsModelItem*				m_parentItem;
	
	// for the moment, copy all the details into separate QVariant variables,
	// which is wasteful, but at least simple...
	
	QVariant			m_enabled;
	QVariant			m_payee;
	QVariant			m_category;
	QVariant			m_amount;
	QVariant			m_frequency;
	QVariant			m_nextDate;
	QVariant			m_account;
	
	// indicies of the scheduled transaction within the document
	unsigned int		m_scheduledTransactionIndex;
};

#endif // SCHEDULED_TRANSACTIONS_VIEW_DATA_MODEL_H
