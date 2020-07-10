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
	m_pRecentDurationDays = new QSpinBox();
	m_pRecentDurationDays->setValue(m_settings.value("transactions/recent_duration_days", 30).toInt());
	
	m_pScrollToLatestTransaction = new QCheckBox();
	m_pScrollToLatestTransaction->setText("Scroll to latest Transaction automatically");
	m_pScrollToLatestTransaction->setChecked(m_settings.value("transactions/scroll_to_latest_auto", true).toBool());
	
	m_pNewTransactionsAreMarkedCleared = new QCheckBox();
	m_pNewTransactionsAreMarkedCleared->setText("New Transactions are marked cleared by default");
	m_pNewTransactionsAreMarkedCleared->setChecked(m_settings.value("transactions/new_transactions_are_marked_cleared", true).toBool());

	//

	m_pFormLayout->addRow("Recent transaction days:", m_pRecentDurationDays);
	m_pFormLayout->addRow("", m_pScrollToLatestTransaction);
	m_pFormLayout->addRow("", m_pNewTransactionsAreMarkedCleared);
}

TransactionsPage::~TransactionsPage()
{
	
}

void TransactionsPage::saveSettings()
{
	m_settings.setValue("transactions/recent_duration_days", m_pRecentDurationDays->value());
	m_settings.setValue("transactions/scroll_to_latest_auto", m_pScrollToLatestTransaction->isChecked());
	m_settings.setValue("transactions/new_transactions_are_marked_cleared", m_pNewTransactionsAreMarkedCleared->isChecked());
}
