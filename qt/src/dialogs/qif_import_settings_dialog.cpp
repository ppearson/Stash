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

#include "qif_import_settings_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

#include <QFormLayout>

QIFImportSettingsDialog::QIFImportSettingsDialog(QWidget* parent, const std::string& fileName,
												 const std::string& dateSample) : QDialog(parent)
{
	resize(527, 302);
	
	setWindowTitle("QIF Import Settings");

	m_pButtonBox = new QDialogButtonBox(this);
	m_pButtonBox->setGeometry(QRect(10, 260, 511, 32));
	m_pButtonBox->setOrientation(Qt::Horizontal);
	m_pButtonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	m_pButtonBox->button(QDialogButtonBox::Ok)->setText("Import");
	
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
	m_pButtonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
	
	QFormLayout* pLayout = new QFormLayout(this);
	setLayout(pLayout);
	
	m_pDescriptionLabel = new QLabel(this);
	
	QString description = "Please select the date format to use when interpreting dates in the file\n'";
	description += fileName.c_str();
	description += "',\n";
	description += "as well as other settings.\n";
	m_pDescriptionLabel->setText(description);
	
	m_pSampleDate = new QLabel(this);
	m_pSampleDate->setText(dateSample.c_str());
	
	m_pDateFormat = new QComboBox(this);
	m_pDateFormat->addItem("dd/mm/YYYY");
	m_pDateFormat->addItem("dd/mm/YY");
	m_pDateFormat->addItem("dd-mm-YYYY");
	m_pDateFormat->addItem("dd-mm-YY");
	m_pDateFormat->addItem("mm/dd/YY");
	m_pDateFormat->addItem("mm/dd/YYYY");
	m_pDateFormat->addItem("mm-dd-YY");
	m_pDateFormat->addItem("mm-dd-YYYY");
	
	m_pMarkImportedTransactionsAsCleared = new QCheckBox(this);
	m_pMarkImportedTransactionsAsCleared->setText("Mark imported Transactions as Cleared");	
	
	pLayout->addRow(new QLabel(this)); // empty row to make layout a bit less compressed, and look nicer...
	pLayout->addRow(m_pDescriptionLabel);
	pLayout->addRow("Sample date:", m_pSampleDate);
	pLayout->addRow("Date format:", m_pDateFormat);
	pLayout->addRow(nullptr, m_pMarkImportedTransactionsAsCleared);
	
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(OKClicked()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void QIFImportSettingsDialog::OKClicked()
{
	int dateFormatIndex = m_pDateFormat->currentIndex();
	
	m_dateFormat = Date::UK;
	
	if (dateFormatIndex > 3)
		m_dateFormat = Date::US;
		
	switch (dateFormatIndex)
	{
		case 0:
		case 1:
		case 4:
		case 5:
			m_separator = '/';
			break;
		case 2:
		case 3:
		case 6:
		case 7:
			m_separator = '-';
			break;
	}
	
	m_markImportedTransactionsAsCleared = m_pMarkImportedTransactionsAsCleared->isChecked();
	
	accept();
}
