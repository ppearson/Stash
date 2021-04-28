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

#include "account_details_dialog.h"

#include <QPushButton>
#include <QToolTip>
#include <QFormLayout>

#include "stash_window.h"

#include "ui_currency_handler.h"

AccountDetailsDialog::AccountDetailsDialog(const StashWindow* pStashWindow, QWidget* parent, bool newAccount) : QDialog(parent),
	m_pStashWindow(pStashWindow)
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
	// TODO: fromUtf8()?
	m_pName->setText(account.getName().c_str());
	
	m_pInstitution->setText(account.getInstitution().c_str());
	m_pNumber->setText(account.getNumber().c_str());
	m_pNote->setText(account.getNote().c_str());
	m_pType->setCurrentIndex((int)account.getType());
}


void AccountDetailsDialog::OKClicked()
{
	m_accountName = m_pName->text().toStdString();
	
	if (m_pStartingBalance && m_pStashWindow)
	{
		double dAmount = 0.0;
		if (!m_pStashWindow->getCurrencyHandler()->parseStringAmountValue(m_pStartingBalance->text(), dAmount))
		{
			// failed to parse, so reject...
			m_pStartingBalance->selectAll();
			m_pStartingBalance->setFocus();
			QToolTip::showText(m_pStartingBalance->mapToGlobal(QPoint(0,0)), "Could not parse Starting Balance string to a currency numeric value.");
			return;
		}
		
		m_accountStartingBalance = dAmount;
	}
	
	m_accountInstitution = m_pInstitution->text().toStdString();
	m_accountNumber = m_pNumber->text().toStdString();
	m_accountNote = m_pNote->text().toStdString();
	m_accountType = (Account::Type)m_pType->currentIndex();
	
	accept();
}
