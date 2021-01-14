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

#ifndef OFX_IMPORT_SETTINGS_DIALOG_H
#define OFX_IMPORT_SETTINGS_DIALOG_H

#include <string>
#include <vector>

#include <QDialog>
#include <QDialogButtonBox>

class QTableWidget;
class QCheckBox;
class QRadioButton;
class QComboBox;
class QLineEdit;

class StashWindow; // annoying, but...

class OFXData;

class OFXImportSettingsDialog : public QDialog
{
	Q_OBJECT
public:
	OFXImportSettingsDialog(QWidget* parent, const StashWindow* mainWindow,
							const OFXData& ofxData);
	
	struct AccountSettings
	{		
		bool			import = true;
		// if this is -1, it's invalid, and we're creating a new account instead
		int				existingAccountIndex = -1;
		// below is for new accounts
		std::string		newAccountName;
		int				newAccountType = 0;
	};
	
	const std::vector<AccountSettings>& getAccountSettings() const
	{
		return m_accountSettings;
	}
	
	bool getReverseTransactionOrder() const
	{
		return m_reverseTransactionOrder;
	}
	
	bool getMarkTransactionsCleared() const
	{
		return m_markTransactionsCleared;
	}
	
	bool getIgnoreExistingTransactions() const
	{
		return m_ignoreExistingTransactions;
	}
	
public slots:
	void OKClicked();
	
	void tableSelectionChanged();
	void cellClicked(int row, int col);
	
	// these modify the local AccountSettings state of the selected item
	void importThisAccountToggled(bool toggleVal);
	
	void importToExistingRBToggled(bool toggleVal);
	void importToNewRBToggled(bool toggleVal);
	
	void existingAccountIndexChanged();
	void newAccountNameChanged();
	void newAccountTypeChanged();
	
protected:
	int getCurrentRow() const;
	
	void updateWidgetsFromSelection();
	
protected:
	const StashWindow*	m_pMainWindow;
	QTableWidget*		m_pTableWidget;
	
	QCheckBox*			m_pImportAccount;
	QRadioButton*		m_pImportToExistingAccountRB;
	QRadioButton*		m_pImportToNewAccountRB;
	
	QComboBox*			m_pExistingAccounts;
	
	QLineEdit*			m_pNewAccountName;
	QComboBox*			m_pNewAccountType;
	
	QCheckBox*			m_pReverseTransactionOrder;
	QCheckBox*			m_pMarkTransactionsCleared;
	QCheckBox*			m_pIgnoreExistingTransactions;
	
	QDialogButtonBox*	m_pButtonBox;
	
	// cached values / state
	bool				m_reverseTransactionOrder;
	bool				m_markTransactionsCleared;
	bool				m_ignoreExistingTransactions;
	
	std::vector<AccountSettings>	m_accountSettings;
};

#endif // OFX_IMPORT_SETTINGS_DIALOG_H
