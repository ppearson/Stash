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

#include "transaction_form_panel.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>

#include <QHBoxLayout>

#include <QGridLayout>

#include "../../core/split_transaction.h"

TransactionFormPanel::TransactionFormPanel(QWidget* parent) : QWidget(parent)
{
	QGridLayout* pGridLayout = new QGridLayout(this);
	pGridLayout->setMargin(4);
		
	QLabel* pPayeeLabel = new QLabel(this);
	pPayeeLabel->setText("&Payee:");
	
	m_pPayee = new QLineEdit(this);
	pPayeeLabel->setBuddy(m_pPayee);
			
	QLabel* pAmountLabel = new QLabel(this);
	pAmountLabel->setText("A&mount:");
	
	m_pAmount = new QLineEdit(this);
	pAmountLabel->setBuddy(m_pAmount);
		
	QLabel* pCategoryLabel = new QLabel(this);
	pCategoryLabel->setText("&Category:");
	
	m_pCategory = new QLineEdit(this);
	pCategoryLabel->setBuddy(m_pCategory);
	
	m_pCleared = new QCheckBox(this);
	m_pCleared->setText("C&leared");
	
	QLabel* pTypeLabel = new QLabel(this);
	pTypeLabel->setText("T&ype:");
	
	m_pType = new QComboBox(this);
	QStringList typeChoices;
	typeChoices << "None" << "Deposit" << "Withdrawal" << "Transfer" << "Standing Order" << "Direct Debit" << "Point Of Sale" <<
				   "ATM" << "Cheque" << "Credit" << "Debit";
	m_pType->addItems(typeChoices);
	m_pType->setCurrentIndex(0);
	pTypeLabel->setBuddy(m_pType);
	
	QLabel* pDescriptionLabel = new QLabel(this);
	pDescriptionLabel->setText("&Description:");
	
	m_pDescription = new QLineEdit(this);
	pDescriptionLabel->setBuddy(m_pDescription);
	
	QLabel* pDateLabel = new QLabel(this);
	pDateLabel->setText("D&ate:");
	
	// create this one intentionally last, such that it's the last tab order (after Description),
	// as DateEdit control isn't really that helpful with tab order, and matches what we did in ObjC
	// Cocoa version...
	m_pDate = new QDateEdit(this);
	// apparently Qt doesn't take the format from the region settings, so...
	m_pDate->setDisplayFormat("dd/MM/yyyy");
	m_pDate->setCalendarPopup(true);
	m_pDate->setDate(QDate::currentDate());
	// ideally Qt would do the right thing based off the system Locale, but obviously not, so...
	m_pDate->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
	pDateLabel->setBuddy(m_pDate);
	
	QPushButton* pUpdateButton = new QPushButton(this);
	pUpdateButton->setText("Update");
	// None of these seem to work in Qt 4.8 or 5.12, it looks like QPushButton
	// has code which only functions correctly if the QPushButton is within a QDialog, so...
//	pUpdateButton->setDefault(true);
//	pUpdateButton->setAutoDefault(true);
	
	connect(pUpdateButton, SIGNAL(clicked()), this, SLOT(updateClicked()));

	m_pPayee->setMinimumHeight(22);
	m_pAmount->setMinimumHeight(22);
	m_pCategory->setMinimumHeight(22);
	m_pType->setMinimumHeight(22);
	m_pDate->setMinimumHeight(22);
	m_pDescription->setMinimumHeight(22);
	
	pGridLayout->addWidget(pPayeeLabel, 0, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pPayee, 0, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pAmountLabel, 0, 2, 1, 1, Qt::AlignRight);
	pGridLayout->addWidget(m_pAmount, 0, 3, 1, 1, 0);
	
	pGridLayout->addWidget(pCategoryLabel, 1, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pCategory, 1, 1, 1, 1, 0);
	
	pGridLayout->addWidget(m_pCleared, 1, 3, 1, 2, Qt::AlignLeft);
	
	pGridLayout->addWidget(pTypeLabel, 2, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pType, 2, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pDateLabel, 2, 2, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pDate, 2, 3, 1, 1, 0);
	
	pGridLayout->addWidget(pDescriptionLabel, 3, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pDescription, 3, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pUpdateButton, 4, 0, 1, 1, Qt::AlignLeft);
	
	
	pGridLayout->setColumnStretch(0, 0);
	pGridLayout->setColumnStretch(1, 50);
	pGridLayout->setColumnStretch(2, 0);
	pGridLayout->setColumnStretch(3, 10);
	
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	setSizePolicy(sizePolicy);
	
//	setMinimumHeight(130);
}

QSize TransactionFormPanel::minimumSizeHint() const
{
	return QSize(400, 180);
}

QSize TransactionFormPanel::sizeHint() const
{
	return QSize(400, 180);
}

void TransactionFormPanel::clear(bool resetDate)
{
	m_pPayee->setText("");
	m_pAmount->setText("0.0");
	m_pCategory->setText(0);
	// TODO: use setting/option for the value?
	m_pCleared->setChecked(true);
	m_pType->setCurrentIndex(0);
	m_pDate->setDate(QDate::currentDate());
	m_pDescription->setText("");
	
	m_pType->setEnabled(true);
	m_pCleared->setEnabled(true);
	m_pDate->setEnabled(true);
}

void TransactionFormPanel::setFocusPayee(bool selectText)
{
	m_pPayee->setFocus();
	if (selectText)
		m_pPayee->selectAll();
}

// TODO: there's a bit of duplication between the below which could be abstracted...

void TransactionFormPanel::setParamsFromTransaction(const Transaction& transaction)
{
	m_pPayee->setText(transaction.getPayee().c_str());
	
	double dAmount = transaction.getAmount().ToDouble();
	
	char szTemp[16];
	// TODO: do this properly...
	sprintf(szTemp, "%0.2f", dAmount);
	
	m_pAmount->setText(szTemp);
	
	m_pCategory->setText(transaction.getCategory().c_str());
	
	m_pCleared->setChecked(transaction.isCleared());
	
	m_pType->setCurrentIndex((unsigned int)transaction.getType());
	
	
	// Date
	QDate date(transaction.getDate().getYear(), transaction.getDate().getMonth(), transaction.getDate().getDay());
	m_pDate->setDate(date);
	
	m_pDescription->setText(transaction.getDescription().c_str());
	
	m_pType->setEnabled(true);
	m_pCleared->setEnabled(true);
	m_pDate->setEnabled(true);
}

void TransactionFormPanel::setParamsFromSplitTransaction(const SplitTransaction& splitTransaction)
{
	m_pPayee->setText(splitTransaction.getPayee().c_str());
	
	double dAmount = splitTransaction.getAmount().ToDouble();
	
	char szTemp[16];
	// TODO: do this properly...
	sprintf(szTemp, "%0.2f", dAmount);
	
	m_pAmount->setText(szTemp);
	
	m_pCategory->setText(splitTransaction.getCategory().c_str());
	
	m_pType->setCurrentIndex(0);
	
	m_pDescription->setText(splitTransaction.getDescription().c_str());
	
	// these don't make sense for split transactions
	m_pType->setEnabled(false);
	m_pCleared->setEnabled(false);
	m_pDate->setEnabled(false);
}

void TransactionFormPanel::setParamsForEmptySplitTransaction(QString amountString)
{
	m_pPayee->setText("Split Value");

	// remove any leading currency symbol.
	// TODO: do this properly, probably best to have the values as an actual fixed and not as a string?
	if (amountString.at(0) == '$')
	{
		amountString = amountString.mid(1);
	}
	m_pAmount->setText(amountString);
	
	m_pCategory->setText("");
	
	m_pType->setCurrentIndex(0);
	
	m_pDescription->setText("Split Value");
	
	// these don't make sense for split transactions
	m_pType->setEnabled(false);
	m_pCleared->setEnabled(false);
	m_pDate->setEnabled(false);
	
	// set focus and select text
	m_pPayee->setFocus();
	m_pPayee->selectAll();
}

void TransactionFormPanel::updateTransactionFromParamValues(Transaction& transaction)
{
	std::string payee = m_pPayee->text().toStdString();
	transaction.setPayee(payee);
	
	double dAmount = m_pAmount->text().toDouble();
	transaction.setAmount(dAmount);
	
	std::string category = m_pCategory->text().toStdString();
	transaction.setCategory(category);
	
	bool cleared = m_pCleared->isChecked();
	transaction.setCleared(cleared);
	
	int typeIndex = m_pType->currentIndex();
	transaction.setType((Transaction::Type)typeIndex);
	
	QDate rawDate = m_pDate->date();
	Date date(rawDate.day(), rawDate.month(), rawDate.year());
	transaction.setDate(date);
	
	std::string description = m_pDescription->text().toStdString();
	transaction.setDescription(description);
}

void TransactionFormPanel::updateSplitTransactionFromParamValues(SplitTransaction& splitTransaction)
{
	std::string payee = m_pPayee->text().toStdString();
	splitTransaction.setPayee(payee);
	
	double dAmount = m_pAmount->text().toDouble();
	splitTransaction.setAmount(dAmount);
	
	std::string category = m_pCategory->text().toStdString();
	splitTransaction.setCategory(category);
	
	std::string description = m_pDescription->text().toStdString();
	splitTransaction.setDescription(description);
}

Date TransactionFormPanel::getEnteredDate() const
{
	QDate rawDate = m_pDate->date();
	Date date(rawDate.day(), rawDate.month(), rawDate.year());
	return date;
}

void TransactionFormPanel::keyPressEvent(QKeyEvent* event)
{
	// Ideally, we wouldn't have to do this, and setting default/autodefault on the Update button
	// would do something useful, but...
	
	// This fires when a widget which doesn't accept the enter key (i.e. QLineEdit) has focus
	// and allows conveniently triggering update with the keyboard...
	// Note: keyPressEvent() is explicitly used over keyReleaseEvent(), as using the latter
	//       suffers all sorts of issues with QDialogs (including QFileDialog) sending stray
	//       events after being closed (I guess they only accept on keyPress, despite QPushButton
	//       apparently handling keyRelease?), which triggers this again.
	if (event->key() == Qt::Key_Return)
	{
		updateClicked();
	}
	
	QWidget::keyPressEvent(event);
}

void TransactionFormPanel::updateClicked()
{
	bool amountIsValid = false;
	double dAmount = m_pAmount->text().toDouble(&amountIsValid);
	
	if (amountIsValid)
	{
	
		// signal that Update has been pressed, which will be caught by TransactionsViewWidget,
		// and it will call either updateTransactionFromParamValues() or updateSplitTransactionFromParamValues()
		// as appropriate...
		
		emit transactionValuesUpdated();
	}
}
