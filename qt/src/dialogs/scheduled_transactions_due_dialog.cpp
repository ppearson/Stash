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

#include "scheduled_transactions_due_dialog.h"

#include <QPushButton>
#include <QTableWidget>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QTableWidgetItem>
#include <QHeaderView>

#include "stash_window.h"

ScheduledTransactionsDueDialog::ScheduledTransactionsDueDialog(StashWindow* pStashWindow,
															   const DueSchedTransactions& dueTransactions)
	: QDialog(pStashWindow),
	  m_pStashWindow(pStashWindow),
	  m_dueTransactions(dueTransactions)
{
	resize(927, 302);
	
	m_pAddTransactionButton = new QPushButton(this);
	m_pAddTransactionButton->setText("Add Transaction");
	
	m_pSkipTransactionButton = new QPushButton(this);
	m_pSkipTransactionButton->setText("Skip");
	
	QPushButton* pCloseButton = new QPushButton(this);
	pCloseButton->setText("Close");
	
	connect(m_pAddTransactionButton, SIGNAL(clicked()), this, SLOT(addTransactionClicked()));
	connect(m_pSkipTransactionButton, SIGNAL(clicked()), this, SLOT(skipTransactionClicked()));
	connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));

	m_pTableWidget = new QTableWidget(this);
	m_pTableWidget->setAlternatingRowColors(true);
	m_pTableWidget->verticalHeader()->setVisible(false);
	m_pTableWidget->horizontalHeader()->setHighlightSections(false);
	m_pTableWidget->setShowGrid(false);
	m_pTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	m_pTableWidget->setColumnCount(5);
	QStringList headerItems;
	headerItems << "Payee" << "Description" << "Amount" << "Date" << "Account";
	m_pTableWidget->setHorizontalHeaderLabels(headerItems);
	m_pTableWidget->setColumnWidth(0, 190);
	m_pTableWidget->setColumnWidth(1, 280);
	m_pTableWidget->setColumnWidth(2, 120);
	m_pTableWidget->setColumnWidth(3, 100);
	m_pTableWidget->setColumnWidth(4, 130);
	
	QHBoxLayout* mainButtons = new QHBoxLayout();
	mainButtons->addWidget(m_pAddTransactionButton);
	mainButtons->addWidget(m_pSkipTransactionButton);
	mainButtons->addStretch(1);
	mainButtons->addWidget(pCloseButton);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(m_pTableWidget, 1);
	mainLayout->addLayout(mainButtons, 0);
	
	setWindowTitle("Due Scheduled Transactions");
	setAttribute(Qt::WA_DeleteOnClose, true);
	
	// fill in table
	
	m_pTableWidget->setRowCount(m_dueTransactions.transactions.size());

	int rowIndex = 0;
	for (const DueSchedTransactions::DueSchedTrans& trans : m_dueTransactions.transactions)
	{
		m_pTableWidget->setRowHeight(rowIndex, 18);

		QTableWidgetItem* pPayeeCell = new QTableWidgetItem(trans.payee.c_str());
		m_pTableWidget->setItem(rowIndex, 0, pPayeeCell);

		QTableWidgetItem* pDescriptionCell = new QTableWidgetItem(trans.description.c_str());
		m_pTableWidget->setItem(rowIndex, 1, pDescriptionCell);

		QTableWidgetItem* pAmountCell = new QTableWidgetItem(trans.amount);
		pAmountCell->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_pTableWidget->setItem(rowIndex, 2, pAmountCell);

		QTableWidgetItem* pDateCell = new QTableWidgetItem(trans.date.c_str());
		pDateCell->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
		m_pTableWidget->setItem(rowIndex, 3, pDateCell);

		QTableWidgetItem* pAccountCell = new QTableWidgetItem(trans.account.c_str());
		m_pTableWidget->setItem(rowIndex, 4, pAccountCell);

		rowIndex++;
	}
	
	m_pTableWidget->selectRow(0);
}

void ScheduledTransactionsDueDialog::addTransactionClicked()
{
	handleActionButtonClick(eAddTrans);
}

void ScheduledTransactionsDueDialog::skipTransactionClicked()
{
	handleActionButtonClick(eSkipTrans);
}

void ScheduledTransactionsDueDialog::handleActionButtonClick(ActionType type)
{
	QModelIndex index;
	
	QModelIndexList selectedIndices = m_pTableWidget->selectionModel()->selectedIndexes();
	if (!selectedIndices.empty())
	{
		index = selectedIndices[0];
		
		if (index.isValid())
		{
			int selectedDueSchedTransactionIndex = index.row();
			
			if (selectedDueSchedTransactionIndex < (int)m_dueTransactions.transactions.size())
			{
				// now actually look up the index of the due scheduled transaction item, in order to map it back
				// to the index of the scheduled transaction in the main Document class...
				const DueSchedTransactions::DueSchedTrans& dueSchedTrans = m_dueTransactions.transactions[selectedDueSchedTransactionIndex];
				unsigned int masterScheduledtransactionIndex = dueSchedTrans.originalIndex;
				
				bool result = false;
				
				// these actions can't really fail currently, so a bit over-complicated, but we might want to do better
				// validation in the future...
				if (type == eAddTrans)
				{
					result = m_pStashWindow->addScheduledTransactionAsTransaction(masterScheduledtransactionIndex);
				}
				else
				{
					result = m_pStashWindow->skipScheduledTransaction(masterScheduledtransactionIndex);
				}
			
				if (result)
				{
					auto eraseIt = m_dueTransactions.transactions.begin() + selectedDueSchedTransactionIndex;
					m_dueTransactions.transactions.erase(eraseIt);
					
					m_pTableWidget->removeRow(selectedDueSchedTransactionIndex);
					
					if (m_dueTransactions.transactions.empty())
					{
						// we're done, so close the dialog
						close();
					}
					
					// select next one
					if (m_pTableWidget->rowCount() > selectedDueSchedTransactionIndex)
					{
						m_pTableWidget->selectRow(selectedDueSchedTransactionIndex);
					}
					else
					{
						m_pTableWidget->selectRow(0);
					}
				}
			}
			else
			{
				// something went wrong and got out of sync...
			}
		}
	}
}
