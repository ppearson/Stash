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
	
	GraphDrawWidget(QWidget* pParent);
	
	virtual void paintEvent(QPaintEvent* event);
	
	struct PieChartItem
	{
		float			angle;
		
		std::string		title;
		QString			amount;
	};
	
	
	void setPieChartItems(const std::vector<PieChartItem>& items);
	
	
	
protected:
	void drawPieChart(QPainter& painter, QPaintEvent* event);
	
protected:
	GraphType		m_graphType;
	
	// pre-generated stuff
	std::vector<QColor>			m_aPieColours;
	
	std::vector<PieChartItem>	m_aPieChartItems;
	
	int				m_selectedItemIndex;
};

#endif // GRAPH_DRAW_WIDGET_H
