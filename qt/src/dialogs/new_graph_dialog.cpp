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

#include "new_graph_dialog.h"

#include <QButtonGroup>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QCalendarWidget>
#include <QCompleter>

#include <QFormLayout>

#include "../../core/account.h"
#include "../../core/document.h"

NewGraphDialog::NewGraphDialog(const Document& document, QWidget* parent) : QDialog(parent),
	m_document(document)
{
	resize(527, 302);
	
	setWindowTitle("Create New Graph");

	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setGeometry(QRect(10, 260, 511, 32));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	
	QFormLayout* pLayout = new QFormLayout(this);
	setLayout(pLayout);
	
	//
	
	m_pName = new QLineEdit(this);
	pLayout->addRow("Name:", m_pName);
	
	m_pAccount = new QComboBox(this);
	pLayout->addRow("Account:", m_pAccount);
		
	for (const Account& acc : m_document.getAccounts())
	{
		std::string accountName = acc.getName();
		m_pAccount->addItem(accountName.c_str());
	}
	
	m_pIgnoreTransfers = new QCheckBox(this);
	m_pIgnoreTransfers->setText("Ignore Transfers");
	m_pIgnoreTransfers->setChecked(true);
	
	pLayout->addRow("", m_pIgnoreTransfers);
	
	m_pDatesPreset = new QComboBox(this);
	QStringList presetChoices;
	presetChoices << "Last Month" << "Last 3 Months" << "Last Year";
	m_pDatesPreset->addItems(presetChoices);
	
	pLayout->addRow("Dates preset", m_pDatesPreset);
	
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(OKClicked()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));
	
	m_pName->setFocus();
}

void NewGraphDialog::OKClicked()
{
	m_name = m_pName->text().toStdString();
	
	if (m_name.empty())
	{
		m_pName->setFocus();
		return;
	}
	
	m_accountIndex = m_pAccount->currentIndex();
	
	m_ignoreTransfers = m_pIgnoreTransfers->isChecked();
	
	unsigned int datePresetIndex = m_pDatesPreset->currentIndex();
	
	if (datePresetIndex == 0)
	{
		m_endDate.Now();
		m_startDate = m_endDate;
		m_startDate.DecrementDays(30);
	}
	else if (datePresetIndex == 1)
	{
		m_endDate.Now();
		m_startDate = m_endDate;
		m_startDate.DecrementDays(90);
	}
	else if (datePresetIndex == 2)
	{
		m_endDate.Now();
		m_startDate = m_endDate;
		m_startDate.DecrementDays(365);
	}
	
	accept();
}
