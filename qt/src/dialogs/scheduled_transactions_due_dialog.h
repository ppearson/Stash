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

#ifndef SCHEDULED_TRANSACTIONS_DUE_DIALOG_H
#define SCHEDULED_TRANSACTIONS_DUE_DIALOG_H

#include <QDialog>

#include <vector>

class QPushButton;
class QTableWidget;

class StashWindow;

// structures to represent temporary due transaction state while the
// ScheduledTransactionsDueDialog is displayed
struct DueSchedTransactions
{
	struct DueSchedTrans
	{
		DueSchedTrans(unsigned int index, const std::string& payeeVal, const std::string& desc) :
			originalIndex(index),
			payee(payeeVal),
			description(desc)
		{
			
		}
			
		unsigned int		originalIndex = 0;
		std::string			payee;
		std::string			description;
		QString				amount;
		std::string			date;
		std::string			account;
	};
	
	std::vector<DueSchedTrans>	transactions;
};

class ScheduledTransactionsDueDialog : public QDialog
{
	Q_OBJECT
public:
	ScheduledTransactionsDueDialog(StashWindow* pStashWindow, const DueSchedTransactions& dueTransactions);

public slots:
	void addTransactionClicked();
	void skipTransactionClicked();
	
	
protected:
	enum ActionType
	{
		eAddTrans,
		eSkipTrans
	};

	void handleActionButtonClick(ActionType type);
	
private:
	StashWindow*		m_pStashWindow;
	
	
	DueSchedTransactions	m_dueTransactions;
	
	// widgets
	
	QTableWidget*		m_pTableWidget;
	
	QPushButton*		m_pAddTransactionButton;
	QPushButton*		m_pSkipTransactionButton;
	
};

#endif // SCHEDULED_TRANSACTIONS_DUE_DIALOG_H
