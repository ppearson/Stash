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

#include "general_page.h"

#include <QCheckBox>
#include <QComboBox>

// Note: QSettings doesn't seem to like a namespace being "general" in Qt 5 (Qt 4.8 is fine), according to the docs
//       it should cope with this and rename it, but it leads to duplicate inconsistent values
//       being saved, so use a totally different namespace ("global") instead, to avoid the problem.

GeneralPage::GeneralPage(QSettings& settings, QWidget* parent) : SettingsPage(settings, parent)
{
	m_pOpenMostRecentFileStartup = new QCheckBox(this);
	m_pOpenMostRecentFileStartup->setText("Open most recent file on startup");
	m_pOpenMostRecentFileStartup->setChecked(m_settings.value("global/open_most_recent_file_startup", false).toBool());
	
	m_pMakeBackupFileWhenSaving = new QCheckBox(this);
	m_pMakeBackupFileWhenSaving->setText("Make backup file when saving");
	m_pMakeBackupFileWhenSaving->setChecked(m_settings.value("global/make_backup_file_when_saving", true).toBool());
	
	m_pDisplayCurrencySourceType = new QComboBox(this);
	m_pDisplayCurrencySourceType->addItem("Detect from System Locale Settings");
	m_pDisplayCurrencySourceType->addItem("Use per Document override (not implemented yet)");
	m_pDisplayCurrencySourceType->addItem("Manual override");
	
	int displayCurrencySourceTypeValue = m_settings.value("global/display_currency_source_type", 0).toInt();
	m_pDisplayCurrencySourceType->setCurrentIndex(displayCurrencySourceTypeValue);
	
	m_pDisplayCurrencyManualOverride = new QComboBox(this);
	m_pDisplayCurrencyManualOverride->addItem("NZ Dollar (-$1,111.11/$1,111.11)");
	m_pDisplayCurrencyManualOverride->addItem(QString::fromUtf8("UK Pound (-£1,111.11/£1,111.11)"));
	m_pDisplayCurrencyManualOverride->addItem("US Dollar (($1,111.11)/$1,111.11)");
	m_pDisplayCurrencyManualOverride->addItem("QLocale formatting");
	m_pDisplayCurrencyManualOverride->setCurrentIndex(m_settings.value("global/display_currency_manual_override", 0).toInt());
	
	// the signal doesn't trigger in this constructor (even if connected above), so...
	displayCurrencySourceTypeChanged(displayCurrencySourceTypeValue);
	
	connect(m_pDisplayCurrencySourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(displayCurrencySourceTypeChanged(int)));
	
	m_pDisplayDateType = new QComboBox(this);
	m_pDisplayDateType->addItem("Detect from System Locale Settings");
	m_pDisplayDateType->addItem("UK Format (DD/MM/YYYY)");
	m_pDisplayDateType->addItem("US Format (MM/DD/YYYY)");
	m_pDisplayDateType->addItem("ISO 8601 (YYYY-MM-DD)");
	m_pDisplayDateType->addItem("QLocale formatting");
	m_pDisplayDateType->setCurrentIndex(m_settings.value("global/display_date_type", 0).toInt());

	//

	m_pFormLayout->addRow("", m_pOpenMostRecentFileStartup);
	m_pFormLayout->addRow("", m_pMakeBackupFileWhenSaving);
	m_pFormLayout->addRow("Display Currency source:", m_pDisplayCurrencySourceType);
	m_pFormLayout->addRow("Display Currency override:", m_pDisplayCurrencyManualOverride);
	m_pFormLayout->addRow("Display Date type:", m_pDisplayDateType);
}

void GeneralPage::saveSettings()
{
	m_settings.setValue("global/open_most_recent_file_startup", m_pOpenMostRecentFileStartup->isChecked());
	m_settings.setValue("global/make_backup_file_when_saving", m_pMakeBackupFileWhenSaving->isChecked());
	m_settings.setValue("global/display_currency_source_type", m_pDisplayCurrencySourceType->currentIndex());
	m_settings.setValue("global/display_currency_manual_override", m_pDisplayCurrencyManualOverride->currentIndex());
	m_settings.setValue("global/display_date_type", m_pDisplayDateType->currentIndex());
}

void GeneralPage::displayCurrencySourceTypeChanged(int currentIndex)
{
	bool enableOverride = (currentIndex == 2);
	
	m_pDisplayCurrencyManualOverride->setEnabled(enableOverride);
}
