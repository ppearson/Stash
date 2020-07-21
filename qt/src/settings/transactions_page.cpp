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

#include "transactions_page.h"

#include <QCheckBox>
#include <QSpinBox>

TransactionsPage::TransactionsPage(QSettings& settings, QWidget* parent) : SettingsPage(settings, parent)
{
	m_pRecentDurationDays = new QSpinBox(this);
	m_pRecentDurationDays->setValue(m_settings.value("transactions/recent_duration_days", 30).toInt());
	
	m_pScrollToLatestTransaction = new QCheckBox(this);
	m_pScrollToLatestTransaction->setText("Scroll to latest Transaction automatically");
	m_pScrollToLatestTransaction->setChecked(m_settings.value("transactions/scroll_to_latest_auto", true).toBool());
	
	m_pNewTransactionsAreMarkedCleared = new QCheckBox(this);
	m_pNewTransactionsAreMarkedCleared->setText("New Transactions are marked cleared by default");
	m_pNewTransactionsAreMarkedCleared->setChecked(m_settings.value("transactions/new_transactions_are_marked_cleared", true).toBool());
	
	m_pColourNegativeAmountValuesRed = new QCheckBox(this);
	m_pColourNegativeAmountValuesRed->setText("Colour negative Amount values red");
	m_pColourNegativeAmountValuesRed->setChecked(m_settings.value("transactions/colour_negative_amount_values_red", false).toBool());
	
	m_pColourNegativeBalanceValuesRed = new QCheckBox(this);
	m_pColourNegativeBalanceValuesRed->setText("Colour negative Balance values red");
	m_pColourNegativeBalanceValuesRed->setChecked(m_settings.value("transactions/colour_negative_balance_values_red", true).toBool());

	//

	m_pFormLayout->addRow("Recent transaction days:", m_pRecentDurationDays);
	m_pFormLayout->addRow("", m_pScrollToLatestTransaction);
	m_pFormLayout->addRow("", m_pNewTransactionsAreMarkedCleared);
	m_pFormLayout->addRow("", m_pColourNegativeAmountValuesRed);
	m_pFormLayout->addRow("", m_pColourNegativeBalanceValuesRed);	
}

void TransactionsPage::saveSettings()
{
	m_settings.setValue("transactions/recent_duration_days", m_pRecentDurationDays->value());
	m_settings.setValue("transactions/scroll_to_latest_auto", m_pScrollToLatestTransaction->isChecked());
	m_settings.setValue("transactions/new_transactions_are_marked_cleared", m_pNewTransactionsAreMarkedCleared->isChecked());
	m_settings.setValue("transactions/colour_negative_amount_values_red", m_pColourNegativeAmountValuesRed->isChecked());
	m_settings.setValue("transactions/colour_negative_balance_values_red", m_pColourNegativeBalanceValuesRed->isChecked());
}
