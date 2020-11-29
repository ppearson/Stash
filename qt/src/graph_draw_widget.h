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
#include <QPolygonF>

#include "../../core/date.h"
#include "../../core/fixed.h"

class StashWindow;

// TODO: I'm on the fence as to whether it was a good idea or not to have this one widget
//       draw all three chart styles, as opposed to having separate widgets for each style.
//       I don't think it *really* matters, but having the member variable state for all
//       three seems non-ideal, but....

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
	
	// could really just use the struct in core/analysis.h, as it's identical, but...
	struct OverviewChartItem
	{
		MonthYear		m_date;
		fixed			m_income;
		fixed			m_outgoings;
	};
	
	
	void setPieChartItems(const std::vector<PieChartItem>& items, const QString& totalAmount);

	void setAreaChartItems(const std::vector<AreaChartItemValues>& dataItems, const std::vector<MonthYear>& dates, fixed maxValue);
	
	void setOverviewChartItems(const std::vector<OverviewChartItem>& items, fixed maxValue);
	

	virtual void mouseReleaseEvent(QMouseEvent* event);
	
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	
protected slots:
	void menuAddSelectedItem();
	
protected:
	void drawPieChart(QPainter& painter, QPaintEvent* event);
	void drawAreaChart(QPainter& painter, QPaintEvent* event);
	void drawOverviewChart(QPainter& painter, QPaintEvent* event);
	
	void displayPopupMenu(const QPoint& pos);
	
	static QPolygon createArrowPolylineFromRect(const QRect& rectBounds, bool pointLeft);
	
signals:
	void selectedItemAdded(QString stringValue);
	
protected:
	StashWindow*				m_pStashWindow;

	GraphType					m_graphType;
	
	// pre-generated stuff
	std::vector<QColor>			m_aPieColours;
	std::vector<QColor>			m_aAreaColours;
	
	QAction*					m_pMenuAddSelectedItem;

	// data for different chart types.
	// TODO: might make more sense to have these enclosed in structs that we can then just pass in + copy/store...
	std::vector<PieChartItem>	m_aPieChartItems;
	QString						m_pieChartTotalAmount;
	QString						m_pieChartEmptyAmount;

	std::vector<AreaChartItemValues>	m_aAreaChartDataItems;
	std::vector<QString>		m_aAreaChartDates;
	fixed						m_maxAreaValue;
	std::string					m_longestDate;
	// don't really like having this here or being defined at draw time,
	// but it simplifies hit-detection for selection considerably...
	std::vector<QPolygonF>		m_areaItemPolygons;
	
	std::vector<OverviewChartItem>		m_aOverviewChartItems;
	std::vector<QString>		m_aOverviewChartDates; // TODO: we could re-use the AreaChart one
	fixed						m_maxOverviewValue;
	int							m_overviewMonthsToShow;
	int							m_overviewCurrentPage;
	bool						m_overviewMultiplePages;
	QRect						m_overviewPageControlRegions[2];
	
	int							m_selectedItemIndex; // used for Pie and Area types
};

#endif // GRAPH_DRAW_WIDGET_H
