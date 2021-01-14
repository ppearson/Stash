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

#include "ofx_export_settings_dialog.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include "stash_window.h"

OFXExportSettingsDialog::OFXExportSettingsDialog(QWidget* parent, const StashWindow* mainWindow) : QDialog(parent),
    m_pMainWindow(mainWindow)
{
	resize(500, 350);
	
	setFixedSize(size());
	
	setWindowTitle("OFX Export Settings");
	
	m_pTableWidget = new QTableWidget(this);
	
	m_pTableWidget->setGeometry(4, 4, 490, 220);
	
	m_pTableWidget->setAlternatingRowColors(true);
	m_pTableWidget->verticalHeader()->setVisible(false);
	m_pTableWidget->horizontalHeader()->setHighlightSections(false);
	m_pTableWidget->setShowGrid(false);
	m_pTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	m_pTableWidget->setColumnCount(2);
	QStringList headerItems;
	headerItems << "Export" << "Account name";
	m_pTableWidget->setHorizontalHeaderLabels(headerItems);
	m_pTableWidget->setColumnWidth(0, 70);
	m_pTableWidget->setColumnWidth(1, 220);
	
	unsigned int numAccounts = mainWindow->getDocumentController().getDocument().getAccountCount();
	m_pTableWidget->setRowCount(numAccounts);
	
	for (int i = 0; i < numAccounts; i++)
	{		
		m_pTableWidget->setRowHeight(i, 18);
		
		QTableWidgetItem* pExportCell = new QTableWidgetItem("");
		pExportCell->setCheckState(Qt::Checked);
		m_pTableWidget->setItem(i, 0, pExportCell);
		
		const Account& account = mainWindow->getDocumentController().getDocument().getAccount(i);

		QTableWidgetItem* pAccountNameCell = new QTableWidgetItem(account.getName().c_str());
		m_pTableWidget->setItem(i, 1, pAccountNameCell);
	}
	
	QLabel* label = new QLabel(this);
	label->setText("OFX File format version:");
	label->setGeometry(4, 250, 150, 18);
	
	m_pOFXVersion = new QComboBox(this);
	m_pOFXVersion->setGeometry(160, 247, 140, 28);
	m_pOFXVersion->addItem("1.0 (SGML)");
	m_pOFXVersion->addItem("2.0 (XML)");
	
	m_pOFXVersion->setCurrentIndex(1);	
	
	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setGeometry(QRect(10, 310, 480, 32));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setText("Export...");
	
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(OKClicked()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));	
}

void OFXExportSettingsDialog::OKClicked()
{
	int numAccounts = m_pTableWidget->rowCount();
	for (int i = 0; i < numAccounts; i++)
	{
		const QTableWidgetItem* pExportCell = m_pTableWidget->item(i, 0);
		if (pExportCell->checkState() == Qt::CheckState::Checked)
			m_aAccountsToExport.emplace_back((unsigned int)i);
	}

	m_OFXVersion = m_pOFXVersion->currentIndex();
	
	accept();
}
