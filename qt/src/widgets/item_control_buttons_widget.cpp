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

#include "item_control_buttons_widget.h"

#include <QLayout>
#include <QPushButton>

ItemControlButtonsWidget::ItemControlButtonsWidget(Type type, QWidget* pParent) : QWidget(pParent),
	m_type(type),
	m_pAddItemButton(nullptr),
	m_pDeleteItemButton(nullptr),
	m_pSplitItemButton(nullptr),
	m_pMoveUpItemButton(nullptr),
	m_pMoveDownItemButton(nullptr)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
#if QT_VERSION < 0x060000
	layout->setMargin(0);
#else
	layout->setContentsMargins(0, 0, 0, 0);
#endif
	layout->setSpacing(0);
	
	if (m_type == eTransaction)
	{
		m_pAddItemButton = createNewButton(QIcon(":stash/images/add_new.svg"), "Add Transaction");
		m_pDeleteItemButton = createNewButton(QIcon(":stash/images/delete_new.svg"), "Delete Transaction");
		m_pSplitItemButton = createNewButton(QIcon(":stash/images/split.png"), "Split Transaction");
		m_pMoveUpItemButton = createNewButton(QIcon(":stash/images/up_new.svg"), "Move Transaction Up");
		m_pMoveDownItemButton = createNewButton(QIcon(":stash/images/down_new.svg"), "Move Transaction Down");
		
		layout->addWidget(m_pAddItemButton);
		layout->addWidget(m_pDeleteItemButton);
		layout->addWidget(m_pSplitItemButton);
		layout->addWidget(m_pMoveUpItemButton);
		layout->addWidget(m_pMoveDownItemButton);
	}
	else if (m_type == ePayee)
	{
		m_pAddItemButton = createNewButton(QIcon(":stash/images/add_new.svg"), "Add Payee");
		m_pDeleteItemButton = createNewButton(QIcon(":stash/images/delete_new.svg"), "Delete Payee");
	
		layout->addWidget(m_pAddItemButton);
		layout->addWidget(m_pDeleteItemButton);
	}
	else if (m_type == eCategory)
	{
		m_pAddItemButton = createNewButton(QIcon(":stash/images/add_new.svg"), "Add Category");
		m_pDeleteItemButton = createNewButton(QIcon(":stash/images/delete_new.svg"), "Delete Category");
	
		layout->addWidget(m_pAddItemButton);
		layout->addWidget(m_pDeleteItemButton);
	}
	else if (m_type == eScheduledTransaction)
	{
		m_pAddItemButton = createNewButton(QIcon(":stash/images/add_new.svg"), "Add Scheduled Transaction");
		m_pDeleteItemButton = createNewButton(QIcon(":stash/images/delete_new.svg"), "Delete Scheduled Transaction");
	
		layout->addWidget(m_pAddItemButton);
		layout->addWidget(m_pDeleteItemButton);
	}
	else if (m_type == eGraph)
	{
		// TODO: we could add the add button, but we'd then have to cope with re-naming items to make it worthwhile...
		m_pDeleteItemButton = createNewButton(QIcon(":stash/images/delete_new.svg"), "Delete Item");
	
		layout->addWidget(m_pDeleteItemButton);
	}
	
	if (m_pAddItemButton)
	{
		m_pAddItemButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
		connect(m_pAddItemButton, SIGNAL(clicked()), this, SLOT(addClicked()));
	}
	if (m_pDeleteItemButton)
	{
		m_pDeleteItemButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
		connect(m_pDeleteItemButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
	}
	if (m_pSplitItemButton)
	{
		m_pSplitItemButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
		connect(m_pSplitItemButton, SIGNAL(clicked()), this, SLOT(splitClicked()));
	}
	if (m_pMoveUpItemButton)
	{
		m_pMoveUpItemButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
		connect(m_pMoveUpItemButton, SIGNAL(clicked()), this, SLOT(moveUpClicked()));
	}
	if (m_pMoveDownItemButton)
	{
		m_pMoveDownItemButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
		connect(m_pMoveDownItemButton, SIGNAL(clicked()), this, SLOT(moveDownClicked()));
	}
	
	layout->addStretch(5);
}

void ItemControlButtonsWidget::setButtonsEnabled(unsigned int buttonsEnabled)
{
	if (m_pAddItemButton)
	{
		m_pAddItemButton->setEnabled(buttonsEnabled & eBtnAdd);
	}
	if (m_pDeleteItemButton)
	{
		m_pDeleteItemButton->setEnabled(buttonsEnabled & eBtnDelete);
	}
	if (m_pSplitItemButton)
	{
		m_pSplitItemButton->setEnabled(buttonsEnabled & eBtnSplit);
	}
	if (m_pMoveUpItemButton)
	{
		m_pMoveUpItemButton->setEnabled(buttonsEnabled & eBtnUp);
	}
	if (m_pMoveDownItemButton)
	{
		m_pMoveDownItemButton->setEnabled(buttonsEnabled & eBtnDown);
	}
}

void ItemControlButtonsWidget::addClicked()
{
	emit addItemButtonClicked();
}

void ItemControlButtonsWidget::deleteClicked()
{
	emit deleteItemButtonClicked();
}

void ItemControlButtonsWidget::splitClicked()
{
	emit splitItemButtonClicked();
}

void ItemControlButtonsWidget::moveUpClicked()
{
	emit moveUpItemButtonClicked();
}

void ItemControlButtonsWidget::moveDownClicked()
{
	emit moveDownItemButtonClicked();
}

QPushButton* ItemControlButtonsWidget::createNewButton(const QIcon& icon, const QString& tooltip)
{
	QPushButton* pNewButton = new QPushButton(icon, "", this);
	pNewButton->setMaximumWidth(28);
	pNewButton->setMinimumWidth(28);
	pNewButton->setMaximumHeight(28);
	pNewButton->setMinimumHeight(28);
	pNewButton->setToolTip(tooltip);
	
	pNewButton->setStyleSheet("QPushButton { qproperty-iconSize: 22px; };");	
	
	return pNewButton;
}
