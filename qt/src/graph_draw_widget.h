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

#ifndef GRAPH_DRAW_WIDGET_H
#define GRAPH_DRAW_WIDGET_H

#include <string>
#include <vector>

#include <QWidget>
#include <QColor>

#include "../../core/datetime.h"
#include "../../core/fixed.h"

class StashWindow;

class GraphDrawWidget : public QWidget
{
	Q_OBJECT
public:
	
	enum GraphType
	{
		eGraphPieChart,
		eGraphAreaChart,
		eGraphOverviewChart
	};
	
	GraphDrawWidget(StashWindow* pStashWindow, QWidget* pParent);
	
	virtual void paintEvent(QPaintEvent* event);
	
	struct PieChartItem
	{
		float			angle;
		
		std::string		title;
		QString			amount;
	};

	struct AreaChartItemValues
	{
		std::string		title;
		std::vector<double> values;
	};
	
	
	void setPieChartItems(const std::vector<PieChartItem>& items);

	void setAreaChartItems(const std::vector<AreaChartItemValues>& dataItems, const std::vector<MonthYear>& dates, fixed maxValue);
	

	virtual void mouseReleaseEvent(QMouseEvent* event);
	
	
protected:
	void drawPieChart(QPainter& painter, QPaintEvent* event);
	void drawAreaChart(QPainter& painter, QPaintEvent* event);
	
protected:
	StashWindow*				m_pStashWindow;

	GraphType					m_graphType;
	
	// pre-generated stuff
	std::vector<QColor>			m_aPieColours;
	std::vector<QColor>			m_aAreaColours;

	// data for different chart types.
	// TODO: might make more sense to have these enclosed in structs that we can then just pass in + copy/store...
	std::vector<PieChartItem>	m_aPieChartItems;

	std::vector<AreaChartItemValues>	m_aAreaChartDataItems;
	std::vector<QString>		m_aAreaChartDates;
	fixed						m_maxValue;
	std::string					m_longestDate;
	
	int							m_selectedItemIndex;
};

#endif // GRAPH_DRAW_WIDGET_H
