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

#include "graphs_view_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>

#include "../../core/document.h"
#include "../../core/graph.h"
#include "../../core/analysis.h"

#include "stash_window.h"
#include "ui_currency_handler.h"
#include "settings_state.h"

#include "form_panels/graph_form_panel.h"

#include "graph_draw_widget.h"

GraphsViewWidget::GraphsViewWidget(Document& document, QWidget* pParent, StashWindow* mainWindow) : QWidget(pParent),
	m_pMainWindow(mainWindow),
	m_pGraph(nullptr),
	m_pGraphFormPanel(nullptr)
{
	QHBoxLayout* pHBoxLayout = new QHBoxLayout(this);
	pHBoxLayout->setMargin(2);
	
	m_pGraphFormPanel = new GraphFormPanel(document, this, this);
	m_pGraphFormPanel->setMaximumWidth(200);
	
	connect(m_pGraphFormPanel, SIGNAL(graphParamValuesChanged()), this, SLOT(graphParamValuesChanged()));
	connect(m_pGraphFormPanel, SIGNAL(graphParamValuesUpdateApplied()), this, SLOT(graphParamValuesUpdated()));
	
	m_pGraphTotal = new GraphDrawWidget(mainWindow, this);
	m_pGraphOverTime = new GraphDrawWidget(mainWindow, this);
	m_pGraphOverview = new GraphDrawWidget(mainWindow, this);
	
	connect(m_pGraphTotal, SIGNAL(selectedItemAdded(QString)), this, SLOT(selectedItemAdded(QString)));
	connect(m_pGraphOverTime, SIGNAL(selectedItemAdded(QString)), this, SLOT(selectedItemAdded(QString)));
	
	m_pTabBarContainer = new QTabWidget(this);
	m_pTabBarContainer->addTab(m_pGraphTotal, "Total");
	m_pTabBarContainer->addTab(m_pGraphOverTime, "Over Time");
	m_pTabBarContainer->addTab(m_pGraphOverview, "Overview");
	
	connect(m_pTabBarContainer, SIGNAL(currentChanged(int)), this, SLOT(viewIndexChanged()));
	
	pHBoxLayout->addWidget(m_pGraphFormPanel);
	
	pHBoxLayout->addWidget(m_pTabBarContainer);
}

QSize GraphsViewWidget::minimumSizeHint() const
{
	return QSize(10, 10);
}

QSize GraphsViewWidget::sizeHint() const
{
	return QSize(600, 300);
}

void GraphsViewWidget::rebuildFromGraph(bool refreshDatePickerFormat)
{
	if (!m_pGraph)
		return;
	
	m_pGraphFormPanel->setParamsFromGraph(*m_pGraph);
	if (refreshDatePickerFormat)
	{
		m_pGraphFormPanel->refreshDatePickerFormats();
	}
	
	int viewTypeIndex = m_pGraph->getViewType();
	m_pTabBarContainer->setCurrentIndex(viewTypeIndex);
	// need to force the update here so internal member variable is correct...
	viewIndexChanged();
	
	buildGraph();
}

void GraphsViewWidget::buildGraph()
{
	buildPieChartGraph();
	buildAreaChartGraph();
	buildOverviewChartGraph();
}

void GraphsViewWidget::buildPieChartGraph()
{
	const Document& document = m_pMainWindow->getDocumentController().getDocument();
	
	TempGraphParamState tempParamState = m_pGraphFormPanel->getTempGraphParamValues();
	
	if (tempParamState.accountIndex < 0)
		return;
	
	const Account* pAccount = &document.getAccount(tempParamState.accountIndex);
	
	const SettingsState& settingsState = m_pMainWindow->getSettingsState();
	
	int sortType = settingsState.getInt("pie_chart/segment_sort_type", 0);
	PieChartCriteria::PieChartSort ePieChartSort = (PieChartCriteria::PieChartSort)sortType;
	
	int pieSmallerThanValue = -1;
	if (settingsState.getBool("pie_chart/group_items_smaller_than", true))
	{
		pieSmallerThanValue = settingsState.getInt("pie_chart/group_items_smaller_than_size", 4);
	}
	std::string pieGroupSmallerName = settingsState.getInternal().value("pie_chart/group_items_smaller_than_name", "Other").toString().toStdString();
	
	PieChartCriteria pieCriteria(pAccount, tempParamState.startDate, tempParamState.endDate,
								 tempParamState.ignoreTransfers, pieSmallerThanValue, pieGroupSmallerName, ePieChartSort);
	
	pieCriteria.m_itemsType = (Graph::ItemsType)tempParamState.itemType;
	pieCriteria.m_aItems = tempParamState.aItems;
	
	bool expenses = (tempParamState.dataType == TempGraphParamState::eExpenseCategories ||
					 tempParamState.dataType == TempGraphParamState::eExpensePayees);
	bool categories = (tempParamState.dataType == TempGraphParamState::eDepositCategories ||
					   tempParamState.dataType == TempGraphParamState::eExpenseCategories);

	PieChartResults pieResults;
	
	if (!buildPieChartItems(pieCriteria, pieResults, expenses, categories))
		return;
	
	UICurrencyHandler* pCurrencyHandler = m_pMainWindow->getCurrencyHandler();
	
	fixed totalAmount;
	
	std::vector<GraphDrawWidget::PieChartItem> pieItems;
	for (const PieChartItem& itemValues : pieResults.m_aValues)
	{
		GraphDrawWidget::PieChartItem newItem;
		newItem.angle = itemValues.getAngle();
		newItem.title = itemValues.getTitle();
		
		// TODO: now that the GraphDrawWidget is actually doing currency formatting (due to AreaChart requirements)
		//       we could do that in there instead of here
		newItem.amount = pCurrencyHandler->formatCurrencyAmount(itemValues.getAmount());
		
		totalAmount += itemValues.getAmount();
		
		pieItems.emplace_back(newItem);
	}
	
	QString totalAmountString = pCurrencyHandler->formatCurrencyAmount(totalAmount);

	m_pGraphTotal->setPieChartItems(pieItems, totalAmountString);
}

void GraphsViewWidget::buildAreaChartGraph()
{
	const Document& document = m_pMainWindow->getDocumentController().getDocument();

	TempGraphParamState tempParamState = m_pGraphFormPanel->getTempGraphParamValues();

	if (tempParamState.accountIndex < 0)
		return;

	const Account* pAccount = &document.getAccount(tempParamState.accountIndex);

	const SettingsState& settingsState = m_pMainWindow->getSettingsState();

	int areaSmallerThanValue = -1;
	if (settingsState.getBool("area_chart/group_items_smaller_than", true))
	{
		areaSmallerThanValue = settingsState.getInt("area_chart/group_items_smaller_than_size", 2);
	}
	std::string areaGroupSmallerName = settingsState.getInternal().value("area_chart/group_items_smaller_than_name", "Other").toString().toStdString();

	AreaChartCriteria areaCriteria(pAccount, tempParamState.startDate, tempParamState.endDate,
								 tempParamState.ignoreTransfers, areaSmallerThanValue, areaGroupSmallerName);

	areaCriteria.m_itemsType = (Graph::ItemsType)tempParamState.itemType;
	areaCriteria.m_aItems = tempParamState.aItems;

	bool expenses = (tempParamState.dataType == TempGraphParamState::eExpenseCategories ||
					 tempParamState.dataType == TempGraphParamState::eExpensePayees);
	bool categories = (tempParamState.dataType == TempGraphParamState::eDepositCategories ||
					   tempParamState.dataType == TempGraphParamState::eExpenseCategories);

	AreaChartResults areaResults;

	if (!buildAreaChartItems(areaCriteria, areaResults, expenses, categories))
		return;

	// TODO: Check both sizes are equal?
	unsigned int numItems = areaResults.m_aValues.size();

	std::vector<GraphDrawWidget::AreaChartItemValues> areaItems;

	for (unsigned int i = 0; i < numItems; i++)
	{
		const AreaChartItem& sourceAreaItem = areaResults.m_aValues[i];

		GraphDrawWidget::AreaChartItemValues newItem;

		unsigned int numValues = sourceAreaItem.getNumItems();
		for (unsigned int j = 0; j < numValues; j++)
		{
			fixed thisValue = sourceAreaItem.getItemAmount(j);
			newItem.values.emplace_back(thisValue.ToDouble());
		}
		
		newItem.title = sourceAreaItem.getTitle();

		areaItems.emplace_back(newItem);
	}

	m_pGraphOverTime->setAreaChartItems(areaItems, areaResults.m_aDates, areaResults.m_overallMax);
}

void GraphsViewWidget::buildOverviewChartGraph()
{
	const Document& document = m_pMainWindow->getDocumentController().getDocument();
	
	TempGraphParamState tempParamState = m_pGraphFormPanel->getTempGraphParamValues();
	
	if (tempParamState.accountIndex < 0)
		return;
	
	const Account* pAccount = &document.getAccount(tempParamState.accountIndex);
	
	fixed overallMax;
	
	OverviewChartCriteria overviewCriteria(pAccount, tempParamState.startDate, tempParamState.endDate,
										   tempParamState.ignoreTransfers, overallMax);
	
	std::vector<OverviewChartItem> overviewItems;
	
	if (!buildOverviewChartItems(overviewCriteria, overviewItems))
		return;
	
	// TODO: doing this "conversion" is pretty silly given both items are identical...
	
	std::vector<GraphDrawWidget::OverviewChartItem> overviewGraphItems;
	
	for (const OverviewChartItem& item : overviewItems)
	{
		GraphDrawWidget::OverviewChartItem newItem;
		newItem.m_date = item.m_date;
		newItem.m_income = item.m_income;
		newItem.m_outgoings = item.m_outgoings;
		
		overviewGraphItems.emplace_back(newItem);
	}
	
	m_pGraphOverview->setOverviewChartItems(overviewGraphItems, overallMax);
}

void GraphsViewWidget::graphParamValuesChanged()
{
	// update the graph view
	buildGraph();
}

void GraphsViewWidget::graphParamValuesUpdated()
{
	// the graph params have been updated (via clicking Update), so mark document as modified
	m_pMainWindow->setWindowModifiedAndRebuildIndex(false);
}

// Not happy about this way of doing things, but...
void GraphsViewWidget::viewIndexChanged()
{
	int selectedIndex = m_pTabBarContainer->currentIndex();
	m_pGraphFormPanel->setViewTypeIndex(selectedIndex);
}

void GraphsViewWidget::selectedItemAdded(QString itemTitle)
{
	m_pGraphFormPanel->addItemString(itemTitle);
}
