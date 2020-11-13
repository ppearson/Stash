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

#include "date_period_control_buttons_widget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>

DatePeriodControlButtonsWidget::DatePeriodControlButtonsWidget(QWidget* pParent) : QWidget(pParent)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	
	m_pPreviousButton = createNewButton(QIcon(), QString("Decrement both dates to the next period type"));
	m_pPreviousButton->setText("<");
	m_pNextButton = createNewButton(QIcon(), QString("Increment both dates to the next period type"));
	m_pNextButton->setText(">");
	
	m_pType = new QComboBox(this);
	m_pType->addItem("Week");
	m_pType->addItem("Month");
	m_pType->addItem("Year");
	m_pType->addItem("Custom");
	
	m_pType->setMinimumWidth(100);
	
	connect(m_pPreviousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));
	connect(m_pNextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
	connect(m_pType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()));
		
	layout->addWidget(m_pPreviousButton);
	layout->addWidget(m_pType);
	layout->addWidget(m_pNextButton);
	
	layout->addStretch(5);	
}

void DatePeriodControlButtonsWidget::setType(DurationType type)
{
	m_pType->setCurrentIndex((int)type);
}

DatePeriodControlButtonsWidget::DurationType DatePeriodControlButtonsWidget::getType() const
{
	DurationType type = (DurationType)m_pType->currentIndex();
	return type;
}

void DatePeriodControlButtonsWidget::previousClicked()
{
	emit previousButtonClicked();
}

void DatePeriodControlButtonsWidget::nextClicked()
{
	emit nextButtonClicked();
}

void DatePeriodControlButtonsWidget::typeChanged()
{
	bool nextPrevEnabled = getType() != eTypeCustom;
	
	m_pPreviousButton->setEnabled(nextPrevEnabled);
	m_pNextButton->setEnabled(nextPrevEnabled);
	
	emit typeIndexChanged();
}

QPushButton* DatePeriodControlButtonsWidget::createNewButton(const QIcon& icon, const QString& tooltip)
{
	QPushButton* pNewButton = new QPushButton(icon, "", this);
	pNewButton->setMaximumWidth(32);
	pNewButton->setMinimumWidth(32);
	
	pNewButton->setMaximumHeight(32);
	pNewButton->setMinimumHeight(32);
	pNewButton->setToolTip(tooltip);
	
	pNewButton->setStyleSheet("QPushButton { qproperty-iconSize: 22px; };");	
	
	return pNewButton;
}
