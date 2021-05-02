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

#ifndef TRANSACTIONS_VIEW_DATA_MODEL_H
#define TRANSACTIONS_VIEW_DATA_MODEL_H

#include <QAbstractItemModel>
#include <QVector>

#include "view_common.h"

class Account;
class Transaction;
class SplitTransaction;

class StashWindow;

class UICurrencyHandler;
class UIDateHandler;

class SettingsState;

class TransactionsModelItem;

class TransactionsViewDataModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	TransactionsViewDataModel(const SettingsState& settingsState, QWidget* parent, StashWindow* stashWindow);
	virtual ~TransactionsViewDataModel();
	
	void setAccount(Account* pAccount);
	
	void setTransactionsViewDurationType(TransactionsViewDurationType viewType);
	
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	
	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	
	virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

//	virtual bool hasChildren(const QModelIndex& parent) const;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	TransactionsModelItem* getItem(const QModelIndex& index) const;

	void rebuildModelFromAccount();
	void clear();

signals:
	void transactionClearedStateChanged(unsigned int transactionIndex, bool newClearedState);
		
protected:
	const SettingsState&			m_settingsState;
	StashWindow*					m_pStashWindow;
	
	Account*						m_pAccount;
	
	TransactionsModelItem*			m_pRootItem;
	
	
	TransactionsViewDurationType	m_transactionsViewType;
	
	bool							m_valid;
};


class TransactionsModelItem
{
public:
	TransactionsModelItem(TransactionsModelItem* parent = nullptr);
	~TransactionsModelItem();
	
	void extractDetails(const Transaction& transaction, UICurrencyHandler* currencyHandler,
						UIDateHandler* dateHandler, unsigned int transactionIndex);
	
	// these are set separately, because they need to be formatted and calculated
	// externally (at least for the balance)
	void setAmount(const QString& value);
	void setBalance(const QString& value);
	
	void setAmountValueRed(bool amountValueRed)
	{
		m_amountValueRed = amountValueRed;
	}
	
	void setBalanceValueRed(bool balanceValueRed)
	{
		m_balanceValueRed = balanceValueRed;
	}

	TransactionsModelItem* child(int number) const;
	int childCount() const;

	TransactionsModelItem* parent();

	int childNumber() const;

	void addChild(TransactionsModelItem* pChild);
	
	// getters for data
	
	QVariant getCleared() const
	{
		return m_cleared;
	}
	
	QVariant getDate() const
	{
		return m_date;
	}
	
	QVariant getPayee() const
	{
		return m_payee;
	}
	
	QVariant getCategory() const
	{
		return m_category;
	}
	
	QVariant getDescription() const
	{
		return m_description;
	}
	
	QVariant getAmount() const
	{
		return m_amount;
	}
	
	QVariant getBalance() const
	{
		return m_balance;
	}
	
	unsigned int getTransactionIndex() const
	{
		return m_transactionIndex;
	}
	
	int getSplitTransactionIndex() const
	{
		return m_splitTransactionIndex;
	}
	
	bool isAmountValueRed() const
	{
		return m_amountValueRed;
	}
	
	bool isBalanceValueRed() const
	{
		return m_balanceValueRed;
	}

protected:
	QVector<TransactionsModelItem*>		m_childItems;
	TransactionsModelItem*				m_parentItem;
	
	// for the moment, copy all the details into separate QVariant variables,
	// which is wasteful, but at least simple...
	
	QVariant						m_cleared;
	QVariant						m_date;
	QVariant						m_payee;
	QVariant						m_category;
	QVariant						m_description;
	QVariant						m_amount;
	QVariant						m_balance;
	
	// indicies of the transaction and optional split transaction
	// within the account

	unsigned int			m_transactionIndex;
	
	// this is optional (dependent on whether the parent transaction is split,
	// and so its children (including this class) are split transactions.
	// If it is a valid split transaction, the index will be a positive integer
	// (starting from 0 for the first).
	// Negative sentinal values of -1 indicate not a split transaction (so just
	// a normal transaction), and -2 means it's a "remainder" split transaction.
	
	int						m_splitTransactionIndex;
	
	bool					m_amountValueRed;
	bool					m_balanceValueRed;
};

#endif // TRANSACTIONS_VIEW_DATA_MODEL_H
