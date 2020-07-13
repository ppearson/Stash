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

#include "scheduled_transaction_form_panel.h"

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

#include "../../core/document.h"
#include "../../core/scheduled_transaction.h"

ScheduledTransactionFormPanel::ScheduledTransactionFormPanel(const Document& document, QWidget* parent) : QWidget(parent),
	m_document(document)
{
	QGridLayout* pGridLayout = new QGridLayout(this);
	pGridLayout->setMargin(4);
		
	QLabel* pPayeeLabel = new QLabel(this);
	pPayeeLabel->setText("Payee:");
	
	m_pPayee = new QLineEdit(this);
	
	QLabel* pTypeLabel = new QLabel(this);
	pTypeLabel->setText("Type:");
	
	m_pType = new QComboBox(this);
	QStringList typeChoices;
	typeChoices << "None" << "Deposit" << "Withdrawal" << "Transfer" << "Standing Order" << "Direct Debit" << "Point Of Sale" <<
				   "ATM" << "Cheque" << "Credit" << "Debit";
	m_pType->addItems(typeChoices);
	m_pType->setCurrentIndex(0);
	
	QLabel* pCategoryLabel = new QLabel(this);
	pCategoryLabel->setText("Category:");
	
	m_pCategory = new QLineEdit(this);	
	
	QLabel* pFrequencyLabel = new QLabel(this);
	pFrequencyLabel->setText("Frequency:");
	
	m_pFrequency = new QComboBox(this);
	QStringList frequencyChoices;
	frequencyChoices << "Weekly" << "Two Weeks" << "Four Weeks" << "Monthly" << "Two Months" << "Quarterly" << "Anually";
	m_pFrequency->addItems(frequencyChoices);
	m_pFrequency->setCurrentIndex(0);
		
	QLabel* pAmountLabel = new QLabel(this);
	pAmountLabel->setText("Amount:");
	
	m_pAmount = new QLineEdit(this);
	
	QLabel* pConstraintLabel = new QLabel(this);
	pConstraintLabel->setText("Constraint:");
	
	m_pConstraint = new QComboBox(this);
	QStringList constraintChoices;
	constraintChoices << "Exact Day" << "Exact or Next Working Day" << "Last Working Day of Month";
	m_pConstraint->addItems(constraintChoices);
	m_pConstraint->setCurrentIndex(0);	
	
	QLabel* pDescriptionLabel = new QLabel(this);
	pDescriptionLabel->setText("Description:");
	
	m_pDescription = new QLineEdit(this);
	
	QLabel* pAccountLabel = new QLabel(this);
	pAccountLabel->setText("Account:");
	
	m_pAccount = new QComboBox(this);
	updateAccountsList(true);
	
	QLabel* pDateLabel = new QLabel(this);
	pDateLabel->setText("Date:");
	
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
	
	QPushButton* pUpdateButton = new QPushButton(this);
	pUpdateButton->setText("Update");
	// None of these seem to work in Qt 4.8 or 5.12, it looks like QPushButton
	// has code which only functions correctly if the QPushButton is within a QDialog, so...
//	pUpdateButton->setDefault(true);
//	pUpdateButton->setAutoDefault(true);
	
	connect(pUpdateButton, SIGNAL(clicked()), this, SLOT(updateClicked()));

	m_pPayee->setMinimumHeight(22);
	m_pType->setMinimumHeight(22);
	m_pCategory->setMinimumHeight(22);
	m_pFrequency->setMinimumHeight(22);
	m_pAmount->setMinimumHeight(22);
	m_pConstraint->setMinimumHeight(22);
	m_pDescription->setMinimumHeight(22);
	m_pDate->setMinimumHeight(22);
	m_pAccount->setMinimumHeight(22);
	
	pGridLayout->addWidget(pPayeeLabel, 0, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pPayee, 0, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pTypeLabel, 0, 2, 1, 1, Qt::AlignRight);
	pGridLayout->addWidget(m_pType, 0, 3, 1, 1, 0);
	
	pGridLayout->addWidget(pCategoryLabel, 1, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pCategory, 1, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pFrequencyLabel, 1, 2, 1, 1, Qt::AlignRight);
	pGridLayout->addWidget(m_pFrequency, 1, 3, 1, 1, 0);
	
	pGridLayout->addWidget(pAmountLabel, 2, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pAmount, 2, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pConstraintLabel, 2, 2, 1, 1, Qt::AlignRight);
	pGridLayout->addWidget(m_pConstraint, 2, 3, 1, 1, 0);
	
	pGridLayout->addWidget(pDescriptionLabel, 3, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pDescription, 3, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pDateLabel, 3, 2, 1, 1, Qt::AlignRight);
	pGridLayout->addWidget(m_pDate, 3, 3, 1, 1, 0);
	
	pGridLayout->addWidget(pAccountLabel, 4, 0, 1, 1, Qt::AlignLeft);
	pGridLayout->addWidget(m_pAccount, 4, 1, 1, 1, 0);
	
	pGridLayout->addWidget(pUpdateButton, 5, 0, 1, 1, Qt::AlignLeft);
	
	pGridLayout->setColumnStretch(0, 0);
	pGridLayout->setColumnStretch(1, 30);
	pGridLayout->setColumnStretch(2, 0);
	pGridLayout->setColumnStretch(3, 10);
	
	QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	setSizePolicy(sizePolicy);
}

QSize ScheduledTransactionFormPanel::minimumSizeHint() const
{
	return QSize(400, 210);
}

QSize ScheduledTransactionFormPanel::sizeHint() const
{
	return QSize(400, 210);
}

void ScheduledTransactionFormPanel::clear()
{
	m_pPayee->setText("");
	m_pType->setCurrentIndex(0);
	m_pCategory->setText(0);
	m_pFrequency->setCurrentIndex(0);
	m_pAmount->setText("0.0");
	m_pConstraint->setCurrentIndex(0);
	
	m_pDate->setDate(QDate::currentDate());
	m_pDescription->setText("");
	
	m_pAccount->setCurrentIndex(0);
	
	updateAccountsList(true);
}

void ScheduledTransactionFormPanel::setFocusPayee(bool selectText)
{
	m_pPayee->setFocus();
	if (selectText)
		m_pPayee->selectAll();
}

void ScheduledTransactionFormPanel::setParamsFromScheduledTransaction(const ScheduledTransaction& schedTransaction)
{
	m_pPayee->setText(schedTransaction.getPayee().c_str());
	
	double dAmount = schedTransaction.getAmount().ToDouble();
	
	char szTemp[16];
	// TODO: do this properly...
	sprintf(szTemp, "%0.2f", dAmount);
	
	m_pAmount->setText(szTemp);
	
	m_pCategory->setText(schedTransaction.getCategory().c_str());
		
	m_pType->setCurrentIndex((unsigned int)schedTransaction.getType());
	
	
	// Date
	QDate date(schedTransaction.getNextDate().getYear(), schedTransaction.getNextDate().getMonth(), schedTransaction.getNextDate().getDay());
	m_pDate->setDate(date);
	
	m_pDescription->setText(schedTransaction.getDescription().c_str());
	
	updateAccountsList(false);
	
	m_pAccount->setCurrentIndex(schedTransaction.getAccount());
}

void ScheduledTransactionFormPanel::updateScheduledTransactionFromParamValues(ScheduledTransaction& schedTransaction)
{
	std::string payee = m_pPayee->text().toStdString();
	schedTransaction.setPayee(payee);
	
	double dAmount = m_pAmount->text().toDouble();
	schedTransaction.setAmount(dAmount);
	
	std::string category = m_pCategory->text().toStdString();
	schedTransaction.setCategory(category);
	
	
	int typeIndex = m_pType->currentIndex();
	schedTransaction.setType((Transaction::Type)typeIndex);
	
	QDate rawDate = m_pDate->date();
	Date date(rawDate.day(), rawDate.month(), rawDate.year());
	schedTransaction.setNextDate(date);
	
	std::string description = m_pDescription->text().toStdString();
	schedTransaction.setDescription(description);
}


Date ScheduledTransactionFormPanel::getEnteredDate() const
{
	QDate rawDate = m_pDate->date();
	Date date(rawDate.day(), rawDate.month(), rawDate.year());
	return date;
}

void ScheduledTransactionFormPanel::keyPressEvent(QKeyEvent* event)
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

void ScheduledTransactionFormPanel::updateAccountsList(bool selectFirst)
{
	m_pAccount->clear();
	
	const std::vector<Account>& aAccounts = m_document.getAccounts();
	for (const Account& account : aAccounts)
	{
		m_pAccount->addItem(account.getName().c_str());
	}
	
	if (selectFirst)
	{
		m_pAccount->setCurrentIndex(0);
	}
}

void ScheduledTransactionFormPanel::updateClicked()
{
	bool amountIsValid = false;
	double dAmount = m_pAmount->text().toDouble(&amountIsValid);
	
	if (amountIsValid)
	{
	
		// signal that Update has been pressed, which will be caught by TransactionsViewWidget,
		// and it will call either updateScheduledTransactionFromParamValues()
		
		emit scheduledTransactionValuesUpdated();
	}
}
