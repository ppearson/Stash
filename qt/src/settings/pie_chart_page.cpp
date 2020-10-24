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

#include "pie_chart_page.h"

#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>

PieChartPage::PieChartPage(QSettings& settings, QWidget* parent) : SettingsPage(settings, parent)
{
	m_pSegmentSortType = new QComboBox(this);
	m_pSegmentSortType->addItem("Size");
	m_pSegmentSortType->addItem("Name");
	m_pSegmentSortType->setCurrentIndex(m_settings.value("pie_chart/segment_sort_type", 0).toInt());
	
	m_pGroupItemsSmaller = new QCheckBox(this);
	m_pGroupItemsSmaller->setText("Group items smaller than:");
	m_pGroupItemsSmaller->setChecked(m_settings.value("pie_chart/group_items_smaller_than", true).toBool());
	
	m_pGroupItemsSmallerSize = new QSpinBox(this);
	m_pGroupItemsSmallerSize->setValue(m_settings.value("pie_chart/group_items_smaller_than_size", 4).toInt());
	m_pGroupItemsSmallerSize->setMinimum(1);
	m_pGroupItemsSmallerName = new QLineEdit(this);
	m_pGroupItemsSmallerName->setText(m_settings.value("pie_chart/group_items_smaller_than_name", "Other").toString());
	
	if (!m_pGroupItemsSmaller->isChecked())
	{
		m_pGroupItemsSmallerSize->setEnabled(false);
		m_pGroupItemsSmallerName->setEnabled(false);
	}
	
	QHBoxLayout* pGroupItemsSmallerMainLayout = new QHBoxLayout();
	pGroupItemsSmallerMainLayout->addWidget(m_pGroupItemsSmaller);
	pGroupItemsSmallerMainLayout->addWidget(m_pGroupItemsSmallerSize);
	QLabel* pDegreesLabel = new QLabel("degrees into new item named:");
	pGroupItemsSmallerMainLayout->addWidget(pDegreesLabel);
	
	m_pSelectedSegmentsType = new QComboBox(this);
	m_pSelectedSegmentsType->addItem("are selected");
	m_pSelectedSegmentsType->addItem("pop out");
	m_pSelectedSegmentsType->setCurrentIndex(m_settings.value("pie_chart/selected_segments_type", 0).toInt());
	
	m_pFormLayout->addRow("Sort segments by:", m_pSegmentSortType);
	
	m_pFormLayout->addRow(pGroupItemsSmallerMainLayout);
	m_pFormLayout->addRow("", m_pGroupItemsSmallerName);
	
	m_pFormLayout->addRow("Selected segments:", m_pSelectedSegmentsType);
	
	connect(m_pGroupItemsSmaller, SIGNAL(toggled(bool)), this, SLOT(groupSmallerThanCheckToggle()));
}

void PieChartPage::saveSettings()
{
	m_settings.setValue("pie_chart/segment_sort_type", m_pSegmentSortType->currentIndex());
	m_settings.setValue("pie_chart/group_items_smaller_than", m_pGroupItemsSmaller->isChecked());
	m_settings.setValue("pie_chart/group_items_smaller_than_size", m_pGroupItemsSmallerSize->value());
	m_settings.setValue("pie_chart/group_items_smaller_than_name", m_pGroupItemsSmallerName->text());
	m_settings.setValue("pie_chart/selected_segments_type", m_pSelectedSegmentsType->currentIndex());
}

void PieChartPage::groupSmallerThanCheckToggle()
{
	bool enableControls = m_pGroupItemsSmaller->isChecked();
	m_pGroupItemsSmallerSize->setEnabled(enableControls);
	m_pGroupItemsSmallerName->setEnabled(enableControls);
}
