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

#include "make_transfer_dialog.h"

#include <QPushButton>
#include <QCalendarWidget>
#include <QCompleter>

#include <QFormLayout>

#include "../../core/document.h"

MakeTransferDialog::MakeTransferDialog(const Document& document, QWidget* parent) : QDialog(parent),
	m_document(document)
{
	resize(527, 302);

	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setGeometry(QRect(10, 260, 511, 32));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	
	QFormLayout* pLayout = new QFormLayout(this);
	setLayout(pLayout);
	
	m_pFromAccount = new QComboBox(this);
	pLayout->addRow("From Account:", m_pFromAccount);
	
	m_pToAccount = new QComboBox(this);
	pLayout->addRow("To Account:", m_pToAccount);
	
	for (const Account& acc : m_document.getAccounts())
	{
		std::string accountName = acc.getName();
		
		m_pFromAccount->addItem(accountName.c_str());
		m_pToAccount->addItem(accountName.c_str());
	}
	
	m_pAmount = new QLineEdit(this);
	m_pAmount->setText("0.0");
	pLayout->addRow("Amount:", m_pAmount);
	
	m_pDate = new QDateEdit(this);
	// apparently Qt doesn't take the format from the region settings, so...
	m_pDate->setDisplayFormat("dd/MM/yyyy");
	m_pDate->setCalendarPopup(true);
	m_pDate->setDate(QDate::currentDate());
	// ideally Qt would do the right thing based off the system Locale, but obviously not, so...
	m_pDate->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
	
	pLayout->addRow("Date:", m_pDate);
	
	m_pCategory = new QComboBox(this);
	m_pCategory->setEditable(true);
	// add a blank item, so QComboBox doesn't automatically fill in the first item.
	m_pCategory->addItem("");
	std::set<std::string>::const_iterator itCat = m_document.CategoryBegin();
	for (; itCat != m_document.CategoryEnd(); ++itCat)
	{
		m_pCategory->addItem(itCat->c_str());
	}
	// TODO: None of these built-in options are that great, so I guess write our own QCompleter?
//	m_pCategory->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion); // okay
	m_pCategory->completer()->setCompletionMode(QCompleter::InlineCompletion); // this would be almost perfect if it didn't occasionally miss things!
//	m_pCategory->completer()->setCompletionMode(QCompleter::PopupCompletion); // okay
	pLayout->addRow("Category:", m_pCategory);
	
	m_pDescription = new QLineEdit(this);
	pLayout->addRow("Description:", m_pDescription);
	
	m_pMarkTransactionsAsCleared = new QCheckBox(this);
	m_pMarkTransactionsAsCleared->setText("Mark Transactions as cleared");
	m_pMarkTransactionsAsCleared->setChecked(true);
	pLayout->addRow("", m_pMarkTransactionsAsCleared);

	setWindowTitle(tr("Make Transfer Transactions"));
	
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(OKClicked()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void MakeTransferDialog::OKClicked()
{
	m_fromAccountIndex = m_pFromAccount->currentIndex();
	m_toAccountIndex = m_pToAccount->currentIndex();
	
	double dAmount = m_pAmount->text().toDouble();
	m_amount = dAmount;
	
	QDate rawDate = m_pDate->date();
	m_date = Date(rawDate.day(), rawDate.month(), rawDate.year());
	
	m_category = m_pCategory->currentText().toStdString();
	m_description = m_pDescription->text().toStdString();
	
	m_markTransactionsAsCleared = m_pMarkTransactionsAsCleared->isChecked();
	
	accept();
}
