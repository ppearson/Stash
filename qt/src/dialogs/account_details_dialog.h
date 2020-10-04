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

#ifndef ACCOUNT_DETAILS_DIALOG_H
#define ACCOUNT_DETAILS_DIALOG_H

#include <string>

#include <QDialog>
#include <QButtonGroup>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QComboBox>

#include "../../core/account.h"

class AccountDetailsDialog : public QDialog
{
	Q_OBJECT
public:
	AccountDetailsDialog(QWidget* parent, bool newAccount);
	
	void setFromAccount(const Account& account);
	
	const std::string& getAccountName() const
	{
		return m_accountName;
	}
	
	const std::string& getAccountStartingBalance() const
	{
		return m_accountStartingBalance;
	}
	
	const std::string& getAccountInstitution() const
	{
		return m_accountInstitution;
	}
	
	const std::string& getAccountNumber() const
	{
		return m_accountNumber;
	}
	
	const std::string& getAccountNote() const
	{
		return m_accountNote;
	}
	
	Account::Type getAccountType() const
	{
		return m_accountType;
	}
	
signals:

public slots:
	void OKClicked();

protected:
	std::string			m_accountName;
	std::string			m_accountStartingBalance;
	std::string			m_accountInstitution;
	std::string			m_accountNumber;
	std::string			m_accountNote;
	Account::Type		m_accountType;

	QDialogButtonBox*	m_pButtonBox;
	
	QLineEdit*			m_pName;
	QLineEdit*			m_pStartingBalance;
	QLineEdit*			m_pInstitution;
	QLineEdit*			m_pNumber;
	QLineEdit*			m_pNote;
	QComboBox*			m_pType;
};

#endif // ACCOUNT_DETAILS_DIALOG_H
