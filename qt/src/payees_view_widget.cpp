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

#include "payees_view_widget.h"

#include <QListWidget>
#include <QLayout>

#include "stash_window.h"
#include "item_control_buttons_widget.h"

PayeesViewWidget::PayeesViewWidget(QWidget* pParent, StashWindow* mainWindow) : QWidget(pParent),
    m_pMainWindow(mainWindow)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	
	m_pListWidget = new QListWidget(this);
	
	layout->addWidget(m_pListWidget);
	
	m_pItemControlButtons = new ItemControlButtonsWidget(ItemControlButtonsWidget::ePayee, this);
	layout->addWidget(m_pItemControlButtons);
	
	connect(m_pListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
	connect(m_pListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChanged(int)));

	connect(m_pItemControlButtons, SIGNAL(addItemButtonClicked()), this, SLOT(addItemClicked()));
	connect(m_pItemControlButtons, SIGNAL(deleteItemButtonClicked()), this, SLOT(deleteItemClicked()));
}

PayeesViewWidget::~PayeesViewWidget()
{
	
}

void PayeesViewWidget::updatePayeesFromDocument()
{
	m_pListWidget->clear();
	
	const Document& document = m_pMainWindow->getDocumentController().getDocument();
	std::set<std::string>::const_iterator itPayee = document.PayeeBegin();
	std::set<std::string>::const_iterator itPayeeEnd = document.PayeeEnd();
	for (; itPayee != itPayeeEnd; ++itPayee)
	{
		m_pListWidget->addItem(itPayee->c_str());
	}
	
	m_pItemControlButtons->setButtonsEnabled(ItemControlButtonsWidget::eBtnAdd);
}

void PayeesViewWidget::addItemClicked()
{
	int newItemPos = m_pListWidget->count();
	
	m_pListWidget->addItem("");
	
	QListWidgetItem* pItem = m_pListWidget->item(newItemPos);
	// explicitly set the new item to be editable, so it can be edited
	// (we don't want other ones to be editable currently, only new ones).
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	m_pListWidget->setCurrentItem(pItem);
	m_pListWidget->scrollToItem(pItem);
	m_pListWidget->editItem(pItem);
}

void PayeesViewWidget::deleteItemClicked()
{
	QListWidgetItem* pItem = m_pListWidget->currentItem();
	if (pItem)
	{
		std::string currentSelectedPayeeText = pItem->text().toStdString();
		
		m_pMainWindow->getDocumentController().getDocument().deletePayee(currentSelectedPayeeText);
		
		int row = m_pListWidget->row(pItem);
		m_pListWidget->takeItem(row);
		
		// TODO: do we need to delete pItem?
		
		m_pMainWindow->setWindowModified(true);
	}
}

void PayeesViewWidget::itemChanged(QListWidgetItem* item)
{
	if (!item)
		return;
	
	// given that we don't know the *old* text value, there's very little
	// control we have here, so just to the absolute bare minimum...
	// TODO: we could cache the QListWidgetItem* pointer in addItemClicked(),
	//       and compare it here to reliably detect a rename...
	//       But that still wouldn't allow us to detect rename from non-"" values,
	//       that would require us to delete the previous value, so not going to cater
	//       for anything like that here...
	
	std::string newPayeeTextValue = item->text().toStdString();
	
	if (!newPayeeTextValue.empty())
	{
		// assume it's a new value, so just add it as a new Payee value...
		
		m_pMainWindow->getDocumentController().getDocument().addPayee(newPayeeTextValue);
		
		m_pMainWindow->setWindowModified(true);
	}
}

void PayeesViewWidget::currentRowChanged(int currentRow)
{
	unsigned int buttonsEnabled = ItemControlButtonsWidget::eBtnAdd;
	
	if (currentRow != -1)
	{
		buttonsEnabled |= ItemControlButtonsWidget::eBtnDelete;
	}
	
	m_pItemControlButtons->setButtonsEnabled(buttonsEnabled);
}
