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

	m_pType = new QComboBox(this);
	m_pType->addItem("Week");
	m_pType->addItem("Month");
	m_pType->addItem("Year");
	m_pType->addItem("Custom");

	// Try and size the push buttons to be the same height as the combo box so they look aligned, given they're right next to each other...
	int targetSize = m_pType->sizeHint().height();

	m_pPreviousButton = createNewButton(targetSize, QIcon(), QString("Decrement both dates to the next period type"));
	m_pPreviousButton->setText("<");
	m_pNextButton = createNewButton(targetSize, QIcon(), QString("Increment both dates to the next period type"));
	m_pNextButton->setText(">");
	
	m_pType->setMinimumWidth(110);
	
	connect(m_pPreviousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));
	connect(m_pNextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
	connect(m_pType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()));
		
	layout->addWidget(m_pPreviousButton);
	layout->addWidget(m_pType);
	layout->addWidget(m_pNextButton);
	
	layout->addStretch(5);	
}

void DatePeriodControlButtonsWidget::setType(Graph::DateType type)
{
	m_pType->setCurrentIndex((int)type);
}

Graph::DateType DatePeriodControlButtonsWidget::getType() const
{
	Graph::DateType type = (Graph::DateType)m_pType->currentIndex();
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
	bool nextPrevEnabled = getType() != Graph::DateCustom;
	
	m_pPreviousButton->setEnabled(nextPrevEnabled);
	m_pNextButton->setEnabled(nextPrevEnabled);
	
	emit typeIndexChanged();
}

QPushButton* DatePeriodControlButtonsWidget::createNewButton(int size, const QIcon& icon, const QString& tooltip)
{
	QPushButton* pNewButton = new QPushButton(icon, "", this);

	pNewButton->setMaximumWidth(size);
	pNewButton->setMinimumWidth(size);
	
	pNewButton->setMaximumHeight(size);
	pNewButton->setMinimumHeight(size);
	pNewButton->setToolTip(tooltip);
	
	pNewButton->setStyleSheet("QPushButton { qproperty-iconSize: 22px; };");	
	
	return pNewButton;
}
