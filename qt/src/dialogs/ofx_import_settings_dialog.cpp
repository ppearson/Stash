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

#include "ofx_import_settings_dialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

#include "../../core/io_ofx.h"

#include "stash_window.h"
#include "ui_currency_formatter.h"

OFXImportSettingsDialog::OFXImportSettingsDialog(QWidget* parent, const StashWindow* mainWindow,
												 const OFXData& ofxData) : QDialog(parent),
	m_pMainWindow(mainWindow),
	m_reverseTransactionOrder(false),
	m_markTransactionsCleared(true),
	m_ignoreExistingTransactions(true)
{
	resize(600, 480);
	
	setFixedSize(size());
	
	setWindowTitle("OFX Import Settings");
	
	m_pTableWidget = new QTableWidget(this);
	
	m_pTableWidget->setGeometry(4, 4, 590, 220);
	
	m_pTableWidget->setAlternatingRowColors(true);
	m_pTableWidget->verticalHeader()->setVisible(false);
	m_pTableWidget->horizontalHeader()->setHighlightSections(false);
	m_pTableWidget->setShowGrid(false);
	m_pTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	m_pTableWidget->setColumnCount(4);
	QStringList headerItems;
	headerItems << "Import" << "Account name" << "Transactions" << "Balance";
	m_pTableWidget->setHorizontalHeaderLabels(headerItems);
	m_pTableWidget->setColumnWidth(0, 70);
	m_pTableWidget->setColumnWidth(1, 240);
	m_pTableWidget->setColumnWidth(2, 120);
	m_pTableWidget->setColumnWidth(3, 100);

	// set table contents
	
	int numAccounts = ofxData.getResponseCount();
	m_pTableWidget->setRowCount(numAccounts);
	
	UICurrencyFormatter* currencyFormatter = m_pMainWindow->getCurrencyFormatter();
	
	bool documentHasExistingAccounts = m_pMainWindow->getDocumentController().getDocument().getAccountCount() > 0;
	
	for (int i = 0; i < numAccounts; i++)
	{
		const OFXStatementTransactionResponse& statementTransactionResponse = ofxData.getResponse(i);
		const OFXStatementResponse& statementResponse = statementTransactionResponse.getStatementResponse();
		
		m_pTableWidget->setRowHeight(i, 18);
		
		QTableWidgetItem* pImportCell = new QTableWidgetItem("");
		pImportCell->setCheckState(Qt::Checked);
		m_pTableWidget->setItem(i, 0, pImportCell);

		QTableWidgetItem* pAccountNameCell = new QTableWidgetItem(statementResponse.getAccount().getAccountID().c_str());
		m_pTableWidget->setItem(i, 1, pAccountNameCell);
		
		// TODO: thousands separators?
		std::string strNumTransactions = std::to_string(statementResponse.getTransactionCount());
		QTableWidgetItem* pTransactionsCountCell = new QTableWidgetItem(strNumTransactions.c_str());
		pTransactionsCountCell->setTextAlignment(Qt::AlignRight);
		m_pTableWidget->setItem(i, 2, pTransactionsCountCell);
		
		QString balanceStr = currencyFormatter->formatCurrencyAmount(statementResponse.getBalance());
		QTableWidgetItem* pBalanceCell = new QTableWidgetItem(balanceStr);
		pBalanceCell->setTextAlignment(Qt::AlignRight);
		m_pTableWidget->setItem(i, 3, pBalanceCell);
		
		AccountSettings newSettings;
		newSettings.newAccountName = statementResponse.getAccount().getAccountID();
		
		// TODO: newAccountType, can we automatically try and detect the correct value for that?
		
		m_accountSettings.emplace_back(newSettings);
	}
	
	m_pImportAccount = new QCheckBox(this);
	m_pImportAccount->setText("Import this account to:");
	m_pImportAccount->setGeometry(4, 230, 170, 18);
	
	m_pImportToExistingAccountRB = new QRadioButton(this);
	m_pImportToExistingAccountRB->setText("Existing account:");
	m_pImportToExistingAccountRB->setGeometry(185, 234, 130, 18);
	
	m_pImportToNewAccountRB = new QRadioButton(this);
	m_pImportToNewAccountRB->setText("New account:");
	m_pImportToNewAccountRB->setGeometry(185, 274, 130, 18);
	
	m_pExistingAccounts = new QComboBox(this);
	m_pExistingAccounts->setGeometry(320, 230, 200, 30);
	const std::vector<Account>& accounts = m_pMainWindow->getDocumentController().getDocument().getAccounts();
	for (const Account& acc : accounts)
	{
		m_pExistingAccounts->addItem(acc.getName().c_str());
	}
	
	if (!documentHasExistingAccounts)
	{
		m_pImportToExistingAccountRB->setEnabled(false);
		m_pExistingAccounts->setEnabled(false);
	}
	
	m_pNewAccountName = new QLineEdit(this);
	m_pNewAccountName->setGeometry(320, 270, 200, 28);
	
	QLabel* pAccountTypeLabel = new QLabel(this);
	pAccountTypeLabel->setText("Account type:");
	pAccountTypeLabel->setGeometry(212, 300, 120, 22);
	
	m_pNewAccountType = new QComboBox(this);
	m_pNewAccountType->setGeometry(320, 300, 200, 30);
	QStringList typeChoices;
	typeChoices << "Cash" << "Checking" << "Savings" << "Credit Card" << "Investment" << "Asset" << "Liabilities";
	m_pNewAccountType->addItems(typeChoices);
	
	updateWidgetsFromSelection();
	
	
	connect(m_pTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectionChanged()));
	connect(m_pTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(cellClicked(int,int)));
	
	connect(m_pImportAccount, SIGNAL(toggled(bool)), this, SLOT(importThisAccountToggled(bool)));
	
	connect(m_pImportToExistingAccountRB, SIGNAL(toggled(bool)), this, SLOT(importToExistingRBToggled(bool)));
	connect(m_pImportToNewAccountRB, SIGNAL(toggled(bool)), this, SLOT(importToNewRBToggled(bool)));
	
	connect(m_pExistingAccounts, SIGNAL(currentIndexChanged(int)), this, SLOT(existingAccountIndexChanged()));
	connect(m_pNewAccountName, SIGNAL(textChanged(QString)), this, SLOT(newAccountNameChanged()));
	connect(m_pNewAccountType, SIGNAL(currentIndexChanged(int)), this, SLOT(newAccountTypeChanged()));
	
	m_pReverseTransactionOrder = new QCheckBox(this);
	m_pReverseTransactionOrder->setText("Reverse transaction order");
	m_pReverseTransactionOrder->setGeometry(4, 330, 200, 18);
	
	m_pMarkTransactionsCleared = new QCheckBox(this);
	m_pMarkTransactionsCleared->setText("Mark transactions cleared");
	m_pMarkTransactionsCleared->setGeometry(4, 360, 200, 18);
	m_pMarkTransactionsCleared->setChecked(m_markTransactionsCleared);
	
	m_pIgnoreExistingTransactions = new QCheckBox(this);
	m_pIgnoreExistingTransactions->setText("Ignore existing transactions");
	m_pIgnoreExistingTransactions->setGeometry(4, 390, 200, 18);
	m_pIgnoreExistingTransactions->setChecked(m_ignoreExistingTransactions);
	
	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setGeometry(QRect(10, 440, 582, 32));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setText("Import");
	
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(OKClicked()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void OFXImportSettingsDialog::OKClicked()
{
	m_reverseTransactionOrder = m_pReverseTransactionOrder->isChecked();
	m_markTransactionsCleared = m_pMarkTransactionsCleared->isChecked();
	m_ignoreExistingTransactions = m_pIgnoreExistingTransactions->isChecked();
	
	accept();
}

void OFXImportSettingsDialog::tableSelectionChanged()
{
	updateWidgetsFromSelection();
}

void OFXImportSettingsDialog::cellClicked(int row, int col)
{
	if (col == 0)
	{
		// toggle the import checked status
		
		AccountSettings& accSettings = m_accountSettings[row];
		accSettings.import = !accSettings.import;
		
		// only do this if we're the selected row
		if (getCurrentRow() == row)
		{
			m_pImportAccount->setChecked(accSettings.import);
		}
	}
}

void OFXImportSettingsDialog::importThisAccountToggled(bool toggleVal)
{
	int selectedIndex = getCurrentRow();
	AccountSettings& accSettings = m_accountSettings[selectedIndex];
	accSettings.import = toggleVal;
	
	QTableWidgetItem* pItemImportCell = m_pTableWidget->item(selectedIndex, 0);
	pItemImportCell->setCheckState(toggleVal ? Qt::Checked : Qt::Unchecked);
	
	updateWidgetsFromSelection();
}

void OFXImportSettingsDialog::importToExistingRBToggled(bool toggleVal)
{
	int selectedIndex = getCurrentRow();
	if (toggleVal)
	{
		AccountSettings& accSettings = m_accountSettings[selectedIndex];
		accSettings.existingAccountIndex = m_pExistingAccounts->currentIndex();
	}
	
	updateWidgetsFromSelection();
}

void OFXImportSettingsDialog::importToNewRBToggled(bool toggleVal)
{
	int selectedIndex = getCurrentRow();
	
	if (toggleVal)
	{
		AccountSettings& accSettings = m_accountSettings[selectedIndex];
		accSettings.existingAccountIndex = -1;
	}
	
	updateWidgetsFromSelection();
}

void OFXImportSettingsDialog::existingAccountIndexChanged()
{
	int selectedIndex = getCurrentRow();
	AccountSettings& accSettings = m_accountSettings[selectedIndex];
	
	accSettings.existingAccountIndex = m_pExistingAccounts->currentIndex();
}

void OFXImportSettingsDialog::newAccountNameChanged()
{
	int selectedIndex = getCurrentRow();
	AccountSettings& accSettings = m_accountSettings[selectedIndex];
	
	accSettings.newAccountName = m_pNewAccountName->text().toStdString();
}

void OFXImportSettingsDialog::newAccountTypeChanged()
{
	int selectedIndex = getCurrentRow();
	AccountSettings& accSettings = m_accountSettings[selectedIndex];
	
	accSettings.newAccountType = m_pNewAccountType->currentIndex();
}

int OFXImportSettingsDialog::getCurrentRow() const
{
	int selectedIndex = m_pTableWidget->currentRow();
	if (selectedIndex == -1)
		selectedIndex = 0;
	
	return selectedIndex;
}

void OFXImportSettingsDialog::updateWidgetsFromSelection()
{
	int selectedIndex = getCurrentRow();
	
	m_pExistingAccounts->blockSignals(true);
	m_pNewAccountName->blockSignals(true);
	m_pNewAccountType->blockSignals(true);
	
	const AccountSettings& accSettings = m_accountSettings[selectedIndex];
	m_pImportAccount->setChecked(accSettings.import);
	if (accSettings.existingAccountIndex == -1)
	{
		m_pImportToExistingAccountRB->setChecked(false);
		m_pImportToNewAccountRB->setChecked(true);
		
		m_pExistingAccounts->setEnabled(false);
		m_pExistingAccounts->setCurrentIndex(0); // not strictly necessary, but...
		
		m_pNewAccountName->setEnabled(true);
		m_pNewAccountType->setEnabled(true);
		
		m_pNewAccountName->setText(accSettings.newAccountName.c_str());
		m_pNewAccountType->setCurrentIndex(accSettings.newAccountType);
	}
	else
	{
		m_pImportToExistingAccountRB->setChecked(true);
		m_pImportToNewAccountRB->setChecked(false);
		
		m_pExistingAccounts->setEnabled(true);
		m_pExistingAccounts->setCurrentIndex(accSettings.existingAccountIndex);
		
		m_pNewAccountName->setEnabled(false);
		m_pNewAccountType->setEnabled(false);
		
		m_pNewAccountName->setText("");
	}
	
	m_pExistingAccounts->blockSignals(false);
	m_pNewAccountName->blockSignals(false);
	m_pNewAccountType->blockSignals(false);
}
