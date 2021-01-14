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

#ifndef OFX_EXPORT_SETTINGS_DIALOG_H
#define OFX_EXPORT_SETTINGS_DIALOG_H

#include <string>
#include <vector>

#include <QDialog>
#include <QDialogButtonBox>

class QTableWidget;
class QComboBox;

class StashWindow; // annoying, but...

class OFXExportSettingsDialog : public QDialog
{
	Q_OBJECT
public:
	OFXExportSettingsDialog(QWidget* parent, const StashWindow* mainWindow);

	const std::vector<unsigned int>& getAccountsToExport() const
	{
		return m_aAccountsToExport;
	}
	
	unsigned int getOFXVersion() const
	{
		return m_OFXVersion;
	}
	
public slots:
	void OKClicked();
	
protected:
	const StashWindow*	m_pMainWindow;
	QTableWidget*		m_pTableWidget;
	
	QComboBox*			m_pOFXVersion;
	
	QDialogButtonBox*	m_pButtonBox;
	
	// cached values
	std::vector<unsigned int>	m_aAccountsToExport;
	
	unsigned int		m_OFXVersion;
};

#endif // OFX_EXPORT_SETTINGS_DIALOG_H
