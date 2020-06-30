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

#include "account_details_dialog.h"

#include <QPushButton>

#include <QFormLayout>

AccountDetailsDialog::AccountDetailsDialog(QWidget* parent, bool newAccount) : QDialog(parent)
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
	
	m_pName = new QLineEdit(this);
	pLayout->addRow("Name:", m_pName);
	
	if (newAccount)
	{
		m_pStartingBalance = new QLineEdit(this);
		m_pStartingBalance->setText("0.0");
		pLayout->addRow("Starting balance:", m_pStartingBalance);
	}
	
	m_pInstitution = new QLineEdit(this);
	pLayout->addRow("Insitution:", m_pInstitution);
	
	m_pNumber = new QLineEdit(this);
	pLayout->addRow("Number:", m_pNumber);
	
	m_pNote = new QLineEdit(this);
	pLayout->addRow("Note:", m_pNote);
	
	m_pType = new QComboBox(this);
	QStringList typeChoices;
	typeChoices << "Cash" << "Checking" << "Savings" << "Credit Card" << "Investment" << "Asset" << "Liabilities";
	m_pType->addItems(typeChoices);
	m_pType->setCurrentIndex(0);
	pLayout->addRow("Type:", m_pType);
	
	m_pName->setFocus();
	
	if (newAccount)
	{
		setWindowTitle(tr("New Account"));
	}
	else
	{
		setWindowTitle(tr("Account Details"));
	}
	
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(OKClicked()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void AccountDetailsDialog::setFromAccount(const Account& account)
{
	
}


void AccountDetailsDialog::OKClicked()
{
	m_accountName = m_pName->text().toStdString();
	
	if (m_pStartingBalance)
	{
		m_accountStartingBalance = m_pStartingBalance->text().toStdString();
	}
	
	m_accountInstitution = m_pInstitution->text().toStdString();
	m_accountNumber = m_pNumber->text().toStdString();
	m_accountNote = m_pNote->text().toStdString();
	m_accountType = (AccountType)m_pType->currentIndex();
	
	accept();
}
