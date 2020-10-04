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

#include "graphs_view_widget.h"

#include <QHBoxLayout>
#include <QTabWidget>

#include "../../core/document.h"
#include "../../core/graph.h"
#include "../../core/analysis.h"

#include "stash_window.h"
#include "ui_currency_formatter.h"

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
	
	connect(m_pGraphFormPanel, SIGNAL(graphParamValuesUpdated()), this, SLOT(graphParamValuesChanged()));
	
	m_pGraphTotal = new GraphDrawWidget(mainWindow, this);
	m_pGraphOverTime = new GraphDrawWidget(mainWindow, this);
	m_pGraphOverview = new GraphDrawWidget(mainWindow, this);
	
	m_pTabBarContainer = new QTabWidget(this);
	m_pTabBarContainer->addTab(m_pGraphTotal, "Total");
	m_pTabBarContainer->addTab(m_pGraphOverTime, "Over Time");
	m_pTabBarContainer->addTab(m_pGraphOverview, "Overview");
	
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

void GraphsViewWidget::rebuildFromGraph()
{
	if (!m_pGraph)
		return;
	
	m_pGraphFormPanel->setParamsFromGraph(*m_pGraph);
	
	buildGraph();

	int viewTypeIndex = m_pGraph->getViewType();
	m_pTabBarContainer->setCurrentIndex(viewTypeIndex);
}

void GraphsViewWidget::buildGraph()
{
	buildPieChartGraph();
	buildAreaChartGraph();

	
}

void GraphsViewWidget::buildPieChartGraph()
{
	const Document& document = m_pMainWindow->getDocumentController().getDocument();
	
	TempGraphParamState tempParamState = m_pGraphFormPanel->getTempGraphParamValues();
	
	if (tempParamState.accountIndex < 0)
		return;
	
	const Account* pAccount = &document.getAccount(tempParamState.accountIndex);
	
	PieChartCriteria::PieChartSort ePieChartSort = PieChartCriteria::PieChartSortAngle;
	
	// TODO: from Settings...
	int pieSmallerThanValue = 4;
	std::string pieGroupSmallerName = "Other";
	
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
	
	UICurrencyFormatter* pCurrencyFormatter = m_pMainWindow->getCurrencyFormatter();
	
	std::vector<GraphDrawWidget::PieChartItem> pieItems;
	for (const PieChartItem& itemValues : pieResults.m_aValues)
	{
		GraphDrawWidget::PieChartItem newItem;
		newItem.angle = itemValues.getAngle();
		newItem.title = itemValues.getTitle();
		
		newItem.amount = pCurrencyFormatter->formatCurrencyAmount(itemValues.getAmount());
		
		pieItems.emplace_back(newItem);
	}
	
	m_pGraphTotal->setPieChartItems(pieItems);	
}

void GraphsViewWidget::buildAreaChartGraph()
{
	const Document& document = m_pMainWindow->getDocumentController().getDocument();

	TempGraphParamState tempParamState = m_pGraphFormPanel->getTempGraphParamValues();

	if (tempParamState.accountIndex < 0)
		return;

	const Account* pAccount = &document.getAccount(tempParamState.accountIndex);

	// TODO: from Settings...
	int areaSmallerThanValue = 2;
	std::string areaGroupSmallerName = "Other";

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

		areaItems.emplace_back(newItem);
	}

	m_pGraphOverTime->setAreaChartItems(areaItems, areaResults.m_aDates, areaResults.m_overallMax);
}

void GraphsViewWidget::graphParamValuesChanged()
{
	buildGraph();
}
