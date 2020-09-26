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

#include "graph_draw_widget.h"

#include <cmath>

#include <QPainter>

GraphDrawWidget::GraphDrawWidget(QWidget* pParent) : QWidget(pParent)
{
	// TODO: static instead?
	
	m_aPieColours.emplace_back(Qt::green);
	m_aPieColours.emplace_back(Qt::red);
	m_aPieColours.emplace_back(Qt::blue);
	m_aPieColours.emplace_back(QColor(125, 125, 0));
	m_aPieColours.emplace_back(Qt::darkGray);
	m_aPieColours.emplace_back(Qt::yellow);
	
	for (QColor& col : m_aPieColours)
	{
		col = col.lighter(150);
	}
	
	m_selectedItemIndex = -1;
}


void GraphDrawWidget::paintEvent(QPaintEvent* event)
{
	// TODO: draw to QPixmap backbuffer instead of doing full draw each time?
	QPainter painter(this);
	
	painter.setPen(Qt::black);
	painter.drawRect(geometry());
	
	if (m_graphType == eGraphPieChart)
	{
		drawPieChart(painter, event);
	}
}

void GraphDrawWidget::setPieChartItems(const std::vector<PieChartItem>& items)
{
	m_graphType = eGraphPieChart;
	
	m_aPieChartItems = items;
	
	m_selectedItemIndex = -1;
	
	update();
	repaint();
}

void GraphDrawWidget::drawPieChart(QPainter& painter, QPaintEvent* event)
{
	if (m_aPieChartItems.empty())
		return;
	
	painter.setRenderHint(QPainter::Antialiasing);
	
	float smallestExtent = 0.0f;
	
	if (geometry().height() < geometry().width())
		smallestExtent = geometry().height();
	else
		smallestExtent = geometry().width();
	
	float mainRadius = (smallestExtent / 2.0f) - 25.0f;
	
	QPoint centrePoint = geometry().center();
	// Can't use inline QPoint operations apparently...
	QRect mainRadiusRect(QPoint(centrePoint.rx() - mainRadius, centrePoint.ry() - mainRadius),
						 QPoint(centrePoint.rx() + mainRadius, centrePoint.ry() + mainRadius));
	
	
	
	// if we don't use float, we get a gap at the end, as we end up missing 9/10 16ths of the circle...
	// TODO: it's still not a perfect fit with this, so something else must also be going on...
	float lastAngleSixteenthsF = 0.0f;
	
	unsigned int colourIndex = 0;
	
	painter.setPen(Qt::darkGray);
	
	// draw the basic wedges
	for (const PieChartItem& item : m_aPieChartItems)
	{
		float thisAngleF = item.angle * 16.0f;
		int thisAngleI = (int)thisAngleF;
		int lastAngleI = (int)lastAngleSixteenthsF;
		
		painter.setBrush(QBrush(m_aPieColours[colourIndex++]));
		
		if (colourIndex >= m_aPieColours.size())
		{
			colourIndex = 0;
		}
		
		painter.drawPie(mainRadiusRect, lastAngleI, thisAngleI);
		
		lastAngleSixteenthsF += thisAngleF;
	}
	
	// now loop over again, drawing any selection highlights, plus text on top
	
	QFont newFont = font();
	newFont.setPointSize(9);
	setFont(newFont);

	QFontMetrics metrics(font());
	
	int textExtentHeight = metrics.height();
	
	painter.setPen(Qt::black);
	
	lastAngleSixteenthsF = 0.0f;
	float lastAngle = 0.0f;
	for (const PieChartItem& item : m_aPieChartItems)
	{
		float thisAngleF = item.angle * 16.0f;
		
		float midPointAngle = (((lastAngle + item.angle) - lastAngle) / 2.0) + lastAngle;
				
		float midPointRads = midPointAngle * 0.017453f;
		
		float wedgeEdgeCentrePosX = std::cos(midPointRads);
		float wedgeEdgeCentrePosY = std::sin(midPointRads);
		
		QString fullText;
		fullText.sprintf("%s  %s",	item.title.c_str(), item.amount.toStdString().c_str());		
		
		int textExtentWidth = metrics.width(fullText);
				
		float fTextStartPosX = centrePoint.rx() + (wedgeEdgeCentrePosX * mainRadius);
		float fTextStartPosY = centrePoint.ry() - (wedgeEdgeCentrePosY * mainRadius);
		
		fTextStartPosX -= ((float)textExtentWidth / 2.0f);
		fTextStartPosY += ((float)textExtentHeight / 2.0f);
		
		// check to see if we're going to fall off the end - if so, move the text
		// left a bit
		/*
		if ((dStartX + extent.width) >= bounds.size.width)
		{
			dStartX -= (dStartX + extent.width) - bounds.size.width;
		}
		else if (dStartX < bounds.origin.x)
		{
			dStartX = 0.0;
		}
		*/
		
		painter.drawText(QPointF(fTextStartPosX, fTextStartPosY), fullText);
		
		lastAngle += item.angle;
	}
}
