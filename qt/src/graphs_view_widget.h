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

#ifndef GRAPHS_VIEW_WIDGET_H
#define GRAPHS_VIEW_WIDGET_H

#include <QWidget>

#include "../../core/graph.h"

class Document;

class QTabWidget;

class StashWindow;

class GraphFormPanel;
class GraphDrawWidget;

class GraphsViewWidget : public QWidget
{
	Q_OBJECT
public:
	GraphsViewWidget(Document& document, QWidget* pParent, StashWindow* mainWindow);

	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
	void setGraph(Graph* pGraph)
	{
		m_pGraph = pGraph;
	}
	
	void rebuildFromGraph(bool refreshDatePickerFormat = false);
	
	StashWindow* getMainWindow()
	{
		return m_pMainWindow;
	}
	
	
protected:
	void buildGraph();
	void buildPieChartGraph();
	void buildAreaChartGraph();
	void buildOverviewChartGraph();
	
	
protected slots:
	// param values were changed live, without updating
	void graphParamValuesChanged();
	
	// the update button to commit them to the document was clicked
	// from the GraphFormPanel...
	void graphParamValuesUpdated();
	
	// which tab view is active - not really happy about this way of doing it, but...
	void viewIndexChanged();
	
	// when a selected chart item is added from the GraphDrawWidget by the right-click menu
	void selectedItemAdded(QString itemTitle);
		
protected:
	StashWindow*		m_pMainWindow;	
	
	Graph*				m_pGraph;
	
	GraphFormPanel*		m_pGraphFormPanel;
	
	QTabWidget*			m_pTabBarContainer;
	
	GraphDrawWidget*	m_pGraphTotal;
	GraphDrawWidget*	m_pGraphOverTime;
	GraphDrawWidget*	m_pGraphOverview;
};

#endif // GRAPHS_VIEW_WIDGET_H
