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

#ifndef TRANSACTIONS_PAGE_H
#define TRANSACTIONS_PAGE_H

#include "settings_page.h"

class QCheckBox;
class QSpinBox;

class TransactionsPage : public SettingsPage
{
	Q_OBJECT
public:
	TransactionsPage(QSettings& settings, QWidget* parent = 0);
	
	virtual void saveSettings() override;
	
protected:
	QSpinBox*		m_pRecentDurationDays;
	QCheckBox*		m_pScrollToLatestTransaction;
	QCheckBox*		m_pNewTransactionsAreMarkedCleared;
	QCheckBox*		m_pResetEditTransactionDateOnRefresh; // not a great description, but...
	QCheckBox*		m_pColourNegativeAmountValuesRed;
	QCheckBox*		m_pColourNegativeBalanceValuesRed;
	QCheckBox*		m_pEnforceNegativeAmountsOfTransactionsBasedOffType;
	QCheckBox*		m_pEnforceNegativeAmountsOfSplitTransactionsBasedOffParent;
};

#endif // TRANSACTIONS_PAGE_H
