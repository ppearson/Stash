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
	
	m_pGraphTotal = new GraphDrawWidget(this);
	m_pGraphOverTime = new GraphDrawWidget(this);
	m_pGraphOverview = new GraphDrawWidget(this);
	
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
}

void GraphsViewWidget::buildGraph()
{
	buildPieChartGraph();
	

	
}

void GraphsViewWidget::buildPieChartGraph()
{
	const Document& document = m_pMainWindow->getDocumentController().getDocument();
	
	TempGraphParamState tempParamState = m_pGraphFormPanel->getTempGraphParamValues();
	
	if (tempParamState.accountIndex < 0)
		return;
	
	const Account* pAccount = &document.getAccount(tempParamState.accountIndex);
	
	PieChartCriteria::PieChartSort ePieChartSort = PieChartCriteria::PieChartSortAngle;
	
	fixed overallPieTotal = 0.0;
	std::vector<PieChartItem> aPieChartValues;
	
	int pieSmallerThanValue = 4;
	std::string pieGroupSmallerName = "Misc";
	
	PieChartCriteria pieCriteria(pAccount, aPieChartValues, tempParamState.startDate, tempParamState.endDate, overallPieTotal,
								 tempParamState.ignoreTransfers, pieSmallerThanValue, pieGroupSmallerName, ePieChartSort);
	
	pieCriteria.m_itemsType = (Graph::ItemsType)tempParamState.itemType;
	pieCriteria.m_aItems = tempParamState.aItems;
	
	bool expenses = (tempParamState.dataType == TempGraphParamState::eExpenseCategories ||
					 tempParamState.dataType == TempGraphParamState::eExpensePayees);
	bool categories = (tempParamState.dataType == TempGraphParamState::eDepositCategories ||
					   tempParamState.dataType == TempGraphParamState::eExpenseCategories);
	
	if (!buildPieChartItems(pieCriteria, expenses, categories))
		return;
	
	UICurrencyFormatter* pCurrencyFormatter = m_pMainWindow->getCurrencyFormatter();
	
	std::vector<GraphDrawWidget::PieChartItem> pieItems;
	for (PieChartItem& itemValues : aPieChartValues)
	{
		GraphDrawWidget::PieChartItem newItem;
		newItem.angle = itemValues.getAngle();
		newItem.title = itemValues.getTitle();
		
		newItem.amount = pCurrencyFormatter->formatCurrencyAmount(itemValues.getAmount());
		
		pieItems.emplace_back(newItem);
	}
	
	m_pGraphTotal->setPieChartItems(pieItems);
	
	
}

void GraphsViewWidget::graphParamValuesChanged()
{
	buildGraph();
}