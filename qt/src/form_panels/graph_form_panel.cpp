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

#include "graph_form_panel.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>

#include <QVBoxLayout>

#include "../../core/document.h"
#include "../../core/account.h"
#include "../../core/graph.h"
#include "../../core/analysis.h"

#include "item_control_buttons_widget.h"

GraphFormPanel::GraphFormPanel(Document& document, GraphsViewWidget* pGraphsViewWidget, QWidget* parent) : QWidget(parent),
	m_document(document),
	m_pGraph(nullptr),
	m_pGraphsViewWidget(pGraphsViewWidget)
{
	QVBoxLayout* pVBoxLayout = new QVBoxLayout(this);
	pVBoxLayout->setMargin(4);
	
	QLabel* pAccountLabel = new QLabel(this);
	pAccountLabel->setText("Account:");
	
	m_pAccount = new QComboBox(this);
	
	m_pDataType = new QComboBox(this);
	QStringList dataTypeChoices;
	dataTypeChoices << "Expense Categories" << "Expense Payees" << "Deposit Categories" << "Deposit Payees";
	m_pDataType->addItems(dataTypeChoices);
	
	m_pIgnoreTransfers = new QCheckBox(this);
	m_pIgnoreTransfers->setText("Ignore transfers");	
	
	QLabel* pStartDateLabel = new QLabel(this);
	pStartDateLabel->setText("Start date:");
	
	m_pStartDate = new QDateEdit(this);
	m_pStartDate->setDisplayFormat("dd/MM/yyyy");
	m_pStartDate->setCalendarPopup(true);
	// ideally Qt would do the right thing based off the system Locale, but obviously not, so...
	m_pStartDate->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
	
	QLabel* pEndDateLabel = new QLabel(this);
	pEndDateLabel->setText("End date:");
	
	m_pEndDate = new QDateEdit(this);
	m_pEndDate->setDisplayFormat("dd/MM/yyyy");
	m_pEndDate->setCalendarPopup(true);
	// ideally Qt would do the right thing based off the system Locale, but obviously not, so...
	m_pEndDate->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
	
	m_pItemType = new QComboBox(this);
	QStringList itemTypeChoices;
	itemTypeChoices << "All Items" << "All Items excluding:" << "Only Specified:";
	m_pItemType->addItems(itemTypeChoices);
	
	// bundle these two together in another VBox, so there's no gap
	QVBoxLayout* pListSubLayout = new QVBoxLayout(this);
	pListSubLayout->setMargin(0);
	pListSubLayout->setSpacing(0);
	m_pItemsList = new QListWidget(this);
	m_pItemsList->setMinimumHeight(130);
	m_pItemsList->setMaximumHeight(130);
	
	m_pItemControlButtons = new ItemControlButtonsWidget(ItemControlButtonsWidget::eGraph, this);
	
	m_pUpdate = new QPushButton(this);
	m_pUpdate->setText("Update");
	
	connect(m_pAccount, SIGNAL(currentIndexChanged(int)), this, SLOT(paramChanged()));
	connect(m_pDataType, SIGNAL(currentIndexChanged(int)), this, SLOT(paramChanged()));
	connect(m_pIgnoreTransfers, SIGNAL(stateChanged(int)), this, SLOT(paramChanged()));
	connect(m_pStartDate, SIGNAL(dateChanged(QDate)), this, SLOT(paramChanged()));
	connect(m_pEndDate, SIGNAL(dateChanged(QDate)), this, SLOT(paramChanged()));
	connect(m_pItemType, SIGNAL(currentIndexChanged(int)), this, SLOT(paramChanged()));
	
	connect(m_pItemControlButtons, SIGNAL(deleteItemButtonClicked()), this, SLOT(deleteItemClicked()));
	
	connect(m_pUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
	
	pVBoxLayout->addWidget(pAccountLabel);
	pVBoxLayout->addWidget(m_pAccount);
	
	pVBoxLayout->addWidget(m_pDataType);
	pVBoxLayout->addWidget(m_pIgnoreTransfers);
	
	pVBoxLayout->addWidget(pStartDateLabel);
	pVBoxLayout->addWidget(m_pStartDate);
	
	pVBoxLayout->addWidget(pEndDateLabel);
	pVBoxLayout->addWidget(m_pEndDate);
	
	pVBoxLayout->addWidget(m_pItemType);
	
	pListSubLayout->addWidget(m_pItemsList);
	pListSubLayout->addWidget(m_pItemControlButtons);
	
	pVBoxLayout->addLayout(pListSubLayout);
	
	pVBoxLayout->addWidget(m_pUpdate);
	
	pVBoxLayout->addStretch(5);
}

QSize GraphFormPanel::minimumSizeHint() const
{
	return QSize(200, 480);
}

QSize GraphFormPanel::sizeHint() const
{
	return QSize(200, 480);
}

void GraphFormPanel::setParamsFromGraph(const Graph& graph)
{
	updateAccountsList(false);
	
	// really not great, but...
	m_pGraph = const_cast<Graph*>(&graph);
	
	int accountIndex = graph.getAccount();
	if (accountIndex == -1 || accountIndex >= m_document.getAccountCount())
		return;
	
	m_pAccount->setCurrentIndex(accountIndex);
	
	// TODO: this is hacky, and not robust to future changes...
	m_pDataType->setCurrentIndex((int)graph.getType());
	
	m_pIgnoreTransfers->setChecked(graph.getIgnoreTransfers());
	
	const Date& startDate = graph.getStartDate();
	const Date& endDate = graph.getEndDate();
	
	m_pStartDate->setDate(QDate(startDate.getYear(), startDate.getMonth(), startDate.getDay()));
	m_pEndDate->setDate(QDate(endDate.getYear(), endDate.getMonth(), endDate.getDay()));
	
	m_pItemType->setCurrentIndex((int)graph.getItemsType());
	
	m_pItemsList->clear();
	
	for (const std::string& str : graph.getItems())
	{
		m_pItemsList->addItem(str.c_str());
	}
}

void GraphFormPanel::updateGraphFromParamValues()
{
	if (!m_pGraph)
		return;
	
	Graph& graph = *m_pGraph;
	
	TempGraphParamState tempState = getTempGraphParamValues();
	
	graph.setAccount(tempState.accountIndex);
	
	graph.setType((Graph::Type)tempState.dataType);
	
	graph.setIgnoreTransfers(tempState.ignoreTransfers);
	
	graph.setStartDate(tempState.startDate);
	graph.setEndDate(tempState.endDate);
	
	Graph::ItemsType itemsType = (Graph::ItemsType)tempState.itemType;
	graph.setItemsType(itemsType);
	
	Graph::ViewType viewType = (Graph::ViewType)m_viewTypeIndex;
	graph.setViewType(viewType);
		
	emit graphParamValuesUpdateApplied();
}

// Not really happy about this, but...
void GraphFormPanel::setViewTypeIndex(int index)
{
	m_viewTypeIndex = index;
}

void GraphFormPanel::addItemString(QString itemTitle)
{
	m_pItemsList->addItem(itemTitle);
	
	emit graphParamValuesChanged();
}

TempGraphParamState GraphFormPanel::getTempGraphParamValues() const
{
	TempGraphParamState state;
	
	state.accountIndex = m_pAccount->currentIndex();
	
	state.dataType = (TempGraphParamState::DataType)m_pDataType->currentIndex();
	
	QDate rawStartDate = m_pStartDate->date();
	QDate rawEndDate = m_pEndDate->date();
	state.startDate = Date(rawStartDate.day(), rawStartDate.month(), rawStartDate.year());
	state.endDate = Date(rawEndDate.day(), rawEndDate.month(), rawEndDate.year());
	
	state.ignoreTransfers = m_pIgnoreTransfers->isChecked();
	
	state.itemType = (TempGraphParamState::ItemType)m_pItemType->currentIndex();
	
	int numItems = m_pItemsList->count();
	for (int i = 0; i < numItems; i++)
	{
		state.aItems.insert(m_pItemsList->item(i)->text().toStdString());
	}
	
	return state;
}

void GraphFormPanel::updateAccountsList(bool selectFirst)
{
	m_pAccount->clear();
	
	const std::vector<Account>& aAccounts = m_document.getAccounts();
	for (const Account& account : aAccounts)
	{
		m_pAccount->addItem(account.getName().c_str());
	}
	
	// Note: This seems to happen regardless of if this code runs or not...
	if (selectFirst)
	{
		m_pAccount->setCurrentIndex(0);
	}
}

void GraphFormPanel::paramChanged()
{
	emit graphParamValuesChanged();
}

void GraphFormPanel::deleteItemClicked()
{
	QListWidgetItem* pItem = m_pItemsList->currentItem();
	if (pItem)
	{
		int row = m_pItemsList->row(pItem);
		m_pItemsList->takeItem(row);
		
		// TODO: delete pItem?
		
		emit graphParamValuesChanged();
	}
}

void GraphFormPanel::updateClicked()
{
	// actually commit the param changes to the core Graph item, and mark the document as modified
	updateGraphFromParamValues();
}
