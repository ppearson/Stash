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

// Note: QSettings doesn't seem to like a namespace being "general" in Qt 5, according to the docs
//       it should cope with this and rename it, but it leads to duplicate inconsistent values
//       being saved, so use a totally different namespace ("global") instead, to avoid the problem.

GeneralPage::GeneralPage(QSettings& settings, QWidget* parent) : SettingsPage(settings, parent)
{
	m_pOpenMostRecentFileStartup = new QCheckBox();
	m_pOpenMostRecentFileStartup->setText("Open most recent file on startup");
	m_pOpenMostRecentFileStartup->setChecked(m_settings.value("global/open_most_recent_file_startup", false).toBool());
	
	m_pMakeBackupFileWhenSaving = new QCheckBox();
	m_pMakeBackupFileWhenSaving->setText("Make backup file when saving");
	m_pMakeBackupFileWhenSaving->setChecked(m_settings.value("global/make_backup_file_when_saving", true).toBool());

	//

	m_pFormLayout->addRow("", m_pOpenMostRecentFileStartup);
	m_pFormLayout->addRow("", m_pMakeBackupFileWhenSaving);
}

GeneralPage::~GeneralPage()
{

}

void GeneralPage::saveSettings()
{
	m_settings.setValue("global/open_most_recent_file_startup", m_pOpenMostRecentFileStartup->isChecked());
	m_settings.setValue("global/make_backup_file_when_saving", m_pMakeBackupFileWhenSaving->isChecked());
}

