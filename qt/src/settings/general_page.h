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

#ifndef GENERAL_PAGE_H
#define GENERAL_PAGE_H

#include "settings_page.h"

class QCheckBox;
class QComboBox;

class GeneralPage : public SettingsPage
{
	Q_OBJECT
public:
	GeneralPage(QSettings& settings, QWidget* parent = 0);

	virtual void saveSettings() override;

public slots:
	void displayCurrencySourceTypeChanged(int currentIndex);

protected:
	QCheckBox*		m_pOpenMostRecentFileStartup;
	QCheckBox*		m_pMakeBackupFileWhenSaving;
	
	QComboBox*		m_pDisplayCurrencySourceType;
	QComboBox*		m_pDisplayCurrencyManualOverride;
	
	QComboBox*		m_pDisplayDateType;
};

#endif // GENERAL_PAGE_H
