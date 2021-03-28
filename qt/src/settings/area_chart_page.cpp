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

#include "area_chart_page.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>

AreaChartPage::AreaChartPage(QSettings& settings, QWidget* parent) : SettingsPage(settings, parent)
{
	m_pGroupItemsSmaller = new QCheckBox(this);
	m_pGroupItemsSmaller->setText("Group items smaller than:");
	m_pGroupItemsSmaller->setChecked(m_settings.value("area_chart/group_items_smaller_than", true).toBool());

	m_pGroupItemsSmallerSize = new QSpinBox(this);
	m_pGroupItemsSmallerSize->setValue(m_settings.value("area_chart/group_items_smaller_than_size", 2).toInt());
	m_pGroupItemsSmallerSize->setMinimum(1);
	m_pGroupItemsSmallerName = new QLineEdit(this);
	m_pGroupItemsSmallerName->setText(m_settings.value("area_chart/group_items_smaller_than_name", "Other").toString());

	if (!m_pGroupItemsSmaller->isChecked())
	{
		m_pGroupItemsSmallerSize->setEnabled(false);
		m_pGroupItemsSmallerName->setEnabled(false);
	}

	QHBoxLayout* pGroupItemsSmallerMainLayout = new QHBoxLayout();
	pGroupItemsSmallerMainLayout->addWidget(m_pGroupItemsSmaller);
	pGroupItemsSmallerMainLayout->addWidget(m_pGroupItemsSmallerSize);
	QLabel* pSizeLabel = new QLabel("% max value into new item named:");
	pGroupItemsSmallerMainLayout->addWidget(pSizeLabel);
	pGroupItemsSmallerMainLayout->addStretch(5);

	m_pFormLayout->addRow(pGroupItemsSmallerMainLayout);
	m_pFormLayout->addRow("", m_pGroupItemsSmallerName);

	connect(m_pGroupItemsSmaller, SIGNAL(toggled(bool)), this, SLOT(groupSmallerThanCheckToggle()));
}

void AreaChartPage::saveSettings()
{
	m_settings.setValue("area_chart/group_items_smaller_than", m_pGroupItemsSmaller->isChecked());
	m_settings.setValue("area_chart/group_items_smaller_than_size", m_pGroupItemsSmallerSize->value());
	m_settings.setValue("area_chart/group_items_smaller_than_name", m_pGroupItemsSmallerName->text());
}

void AreaChartPage::groupSmallerThanCheckToggle()
{
	bool enableControls = m_pGroupItemsSmaller->isChecked();
	m_pGroupItemsSmallerSize->setEnabled(enableControls);
	m_pGroupItemsSmallerName->setEnabled(enableControls);
}
