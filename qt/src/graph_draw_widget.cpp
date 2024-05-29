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

#include <QDate>
#include <QPainter>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>

#include "stash_window.h"
#include "ui_currency_handler.h"

GraphDrawWidget::GraphDrawWidget(StashWindow* pStashWindow, QWidget* pParent) : QWidget(pParent), m_pStashWindow(pStashWindow)
{
	// TODO: static instead?
	
	m_aPieColours.emplace_back(Qt::green);
	m_aPieColours.emplace_back(Qt::red);
	m_aPieColours.emplace_back(QColor(153, 102, 51));
	m_aPieColours.emplace_back(Qt::blue);
	m_aPieColours.emplace_back(QColor(255, 127, 0));
	m_aPieColours.emplace_back(Qt::darkGray);
	m_aPieColours.emplace_back(Qt::yellow);
	
	for (QColor& col : m_aPieColours)
	{
		// Area chart colours need to be slightly transparent so we can see the gridlines underneath
		QColor areaColour = col;
		areaColour.setAlpha(127);

		m_aAreaColours.emplace_back(areaColour);

		// Pie chart colours don't have alpha (don't need it, as nothing underneath), so we make the original colours
		// slightly lighter instead, i.e. more pastel shades
		col = col.lighter(145);
	}
	
	m_selectedItemIndex = -1;
	
	m_pMenuAddSelectedItem = new QAction(this);
	m_pMenuAddSelectedItem->setText("Add Selected Item");
	
	connect(m_pMenuAddSelectedItem, SIGNAL(triggered()), this, SLOT(menuAddSelectedItem()));
	
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void GraphDrawWidget::paintEvent(QPaintEvent* event)
{
	// TODO: draw to QPixmap backbuffer instead of doing full draw each time?
	QPainter painter(this);
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(Qt::black);
	painter.drawRect(geometry());
	
//	fprintf(stderr, "G: (%u, %u, %u, %u)\n", geometry().top(), geometry().left(), geometry().bottom(), geometry().right());
	
	if (m_graphType == eGraphPieChart)
	{
		drawPieChart(painter, event);
	}
	else if (m_graphType == eGraphAreaChart)
	{
		drawAreaChart(painter, event);
	}
	else if (m_graphType == eGraphOverviewChart)
	{
		drawOverviewChart(painter, event);
	}
}

void GraphDrawWidget::setPieChartItems(const std::vector<PieChartItem>& items, const QString& totalAmount)
{
	m_graphType = eGraphPieChart;
	
	m_aPieChartItems = items;
	m_pieChartTotalAmount = totalAmount;
	
	m_selectedItemIndex = -1;
	
	fixed zero;
	UICurrencyHandler* currencyHandler = m_pStashWindow->getCurrencyHandler();
	m_pieChartEmptyAmount = currencyHandler->formatCurrencyAmount(zero);
}

void GraphDrawWidget::setAreaChartItems(const std::vector<AreaChartItemValues>& dataItems, const std::vector<MonthYear>& dates, fixed maxValue)
{
	m_graphType = eGraphAreaChart;

	m_aAreaChartDataItems = dataItems;
	m_aAreaChartDates.clear();
	// format the dates
	for (const MonthYear& my : dates)
	{
		QDate date(my.getYear(), my.getMonth(), 1);

		QString formattedDate = date.toString("MMM\nyyyy");
		m_aAreaChartDates.emplace_back(formattedDate);
	}

	// work out the longest date
	// for the moment, just hard-code September
	// TODO: do we even need this now?
	m_longestDate = "September";

	m_maxAreaValue = maxValue;

	m_selectedItemIndex = -1;
}

void GraphDrawWidget::setOverviewChartItems(const std::vector<OverviewChartItem>& items, fixed maxValue)
{
	m_graphType = eGraphOverviewChart;
	
	m_aOverviewChartDates.clear();
	// format the dates
	for (const OverviewChartItem& item : items)
	{
		QDate date(item.m_date.getYear(), item.m_date.getMonth(), 1);

		QString formattedDate = date.toString("MMM\nyyyy");
		m_aOverviewChartDates.emplace_back(formattedDate);
	}
	
	m_aOverviewChartItems = items;
	m_maxOverviewValue = maxValue;
	
	m_overviewMonthsToShow = 0;
	m_overviewCurrentPage = 0;
	m_overviewMultiplePages = false;
}

void GraphDrawWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton)
	{
		displayPopupMenu(event->pos());
		return;
	}
	
	if (event->button() != Qt::LeftButton)
		return;

	bool earlyOut = false;

	if (m_graphType == eGraphPieChart)
	{
		if (m_aPieChartItems.empty())
		{
			m_selectedItemIndex = -1;
			update();
			repaint();
			return;
		}

		float smallestExtent = 0.0f;

		if (geometry().height() < geometry().width())
			smallestExtent = geometry().height();
		else
			smallestExtent = geometry().width();

		float mainRadius = (smallestExtent / 2.0f) - 25.0f;

		QPoint centrePoint = geometry().center();

		float deltaX = event->pos().x() - centrePoint.x();
		float deltaY = event->pos().y() - centrePoint.y();

		float hitPosRadius = std::sqrt(deltaY * deltaY + deltaX * deltaX);

		if (hitPosRadius > mainRadius)
		{
			m_selectedItemIndex = -1;
			earlyOut = true;
		}

		if (!earlyOut)
		{
			float angleRads = std::atan2(deltaX, deltaY);
			float angleDegs = angleRads * 57.2958f;

			// compensate for differing start position when drawing (starts at 90 deg (3 o'clock)
			angleDegs -= 90.0f;

			if (angleDegs < 0.0f)
				angleDegs += 360.0f;

			float cumulativeAngle = 0.0f;
			int index = 0;
			for (const PieChartItem& item : m_aPieChartItems)
			{
				float endAngle = cumulativeAngle + item.angle;

				if (angleDegs > cumulativeAngle && angleDegs < endAngle)
				{
					m_selectedItemIndex = index;
					break;
				}

				cumulativeAngle += item.angle;
				index++;
			}
		}
	}
	else if (m_graphType == eGraphAreaChart)
	{
		if (m_aAreaChartDataItems.empty() || m_aAreaChartDataItems[0].values.size() <= 2)
		{
			m_selectedItemIndex = -1;
			update();
			repaint();
			return;
		}
		
		m_selectedItemIndex = -1;
		
		QPointF fPos(event->pos());
		
		// TODO: this can be a bit laggy in some situations, being brute-force...
		//       try and remove empty polygons previously.?
		for (unsigned int i = 0; i < m_areaItemPolygons.size(); i++)
		{
			const QPolygonF& polygon = m_areaItemPolygons[i];
			
			if (polygon.containsPoint(fPos, Qt::OddEvenFill))
			{
				m_selectedItemIndex = i;
				break;
			}
		}
	}
	else if (m_graphType == eGraphOverviewChart)
	{
		if (m_overviewPageControlRegions[0].contains(event->pos()) && m_overviewCurrentPage > 0)
		{
			m_overviewCurrentPage -= 1;
		}
		else if (m_overviewPageControlRegions[1].contains(event->pos()))
		{
			m_overviewCurrentPage += 1;
		}
	}

	update();
	repaint();
}

// not needed... just for testing...
QSize GraphDrawWidget::sizeHint() const
{
	return QSize(400, 400);
}

QSize GraphDrawWidget::minimumSizeHint() const
{
	return QSize(200, 200);
}

void GraphDrawWidget::menuAddSelectedItem()
{
	if (m_selectedItemIndex == -1)
		return;
	
	if (m_graphType == eGraphPieChart)
	{
		const PieChartItem& item = m_aPieChartItems[m_selectedItemIndex];
		QString itemTitle(item.title.c_str());
		
		emit selectedItemAdded(itemTitle);
	}
	else if (m_graphType == eGraphAreaChart)
	{
		const AreaChartItemValues& item = m_aAreaChartDataItems[m_selectedItemIndex];
		QString itemTitle(item.title.c_str());
		
		emit selectedItemAdded(itemTitle);
	}
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
	
	// TODO: use "pie_chart/selected_segments_type" setting to control selection style...
	
	// draw the basic wedges
	for (const PieChartItem& item : m_aPieChartItems)
	{
		float thisAngleF = item.angle * 16.0f;
		int thisAngleI = (int)thisAngleF;
		int lastAngleI = (int)lastAngleSixteenthsF;
		
		painter.setBrush(QBrush(m_aPieColours[colourIndex++]));
		if (colourIndex >= m_aPieColours.size())
		{
			// don't repeat back to the beginning for the moment, so that we never have matching
			// first/last colours.
			// TODO: there's probably a better way of doing this so that we can use the first colour more than once...
			colourIndex = 1;
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
	int itemIndex = 0;
	for (const PieChartItem& item : m_aPieChartItems)
	{
		float thisAngleF = item.angle * 16.0f;
		int thisAngleI = (int)thisAngleF;
		int lastAngleI = (int)lastAngleSixteenthsF;

		if (m_selectedItemIndex != -1 && m_selectedItemIndex == itemIndex++)
		{
			// configure thick selection line for outline, and no brush for no fill colour
			QPen selectionPen(QColor(192, 192, 255), 4, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
			painter.setPen(selectionPen);
			painter.setBrush(Qt::NoBrush);
			painter.drawPie(mainRadiusRect, lastAngleI, thisAngleI);

			painter.setPen(Qt::black);
		}

		float midPointAngle = (((lastAngle + item.angle) - lastAngle) / 2.0) + lastAngle;
		float midPointRads = midPointAngle * 0.017453f;
		
		float wedgeEdgeCentrePosX = std::cos(midPointRads);
		float wedgeEdgeCentrePosY = std::sin(midPointRads);
		
		QString fullText = QString("%1  %2").arg(item.title.c_str()).arg(item.amount.toStdString().c_str());
		
		int textExtentWidth = getTextExtentHelper(metrics, fullText);
				
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
		lastAngleSixteenthsF += thisAngleF;
	}
	
	// draw amount text
	
	QRect amountsRect(QPoint(geometry().left() + 10, geometry().bottom() - 45), QPoint(geometry().left() + 200, geometry().bottom() - 10));
	if (m_selectedItemIndex == -1)
	{
		painter.drawText(amountsRect, "Selected amount: " + m_pieChartEmptyAmount);
	}
	else
	{
		const PieChartItem& item = m_aPieChartItems[m_selectedItemIndex];
		painter.drawText(amountsRect, "Selected amount: " + item.amount);
	}
	
	amountsRect.setTop(amountsRect.top() + textExtentHeight + 5);
	painter.drawText(amountsRect, "Total amount: " + m_pieChartTotalAmount);
}

void GraphDrawWidget::drawAreaChart(QPainter& painter, QPaintEvent* event)
{
	if (m_aAreaChartDataItems.empty() || m_aAreaChartDataItems[0].values.size() <= 2)
		return;

	QFontMetrics metrics(font());

	UICurrencyHandler* currencyHandler = m_pStashWindow->getCurrencyHandler();
	QString maxValueString = currencyHandler->formatCurrencyAmount(m_maxAreaValue);

	float leftMargin = 30.0f + getTextExtentHelper(metrics, maxValueString);
	int bottomMargin = (metrics.height() * 2) + 20;
	int topMargin = 40;

	painter.setRenderHint(QPainter::Antialiasing);

	unsigned int numXValues = m_aAreaChartDataItems[0].values.size();

	QRect plotArea = geometry();
	plotArea.adjust(leftMargin, topMargin, -20, -bottomMargin);

	painter.drawRect(plotArea);

	// try and generate a "nice" number which divides easily
	double ceilMaxValue = std::ceil(std::ceil(m_maxAreaValue.ToDouble() / 10.0) * 10.0);

	float xIncrement = (float)plotArea.width() / (float)(numXValues - 1);
	float YScale = (float)plotArea.height() / (float)ceilMaxValue;

	painter.setPen(Qt::darkGray);

	int dateLabelAlternating = 1;

	// work out how many X-axis labels can fit into the space available
	if (!m_longestDate.empty())
	{
		float dateTextWidth =getTextExtentHelper(metrics, m_longestDate.c_str());
		// slight margin so that we ensure that labels don't touch
		dateTextWidth += 4.0f;

		if (dateTextWidth >= xIncrement)
			dateLabelAlternating = 2;

		if (dateTextWidth >= xIncrement * 2.0f)
			dateLabelAlternating = 3;
	}


	QRect yAxisLabelRect(0, plotArea.bottom() + 10, 40, 30);
	// draw x gridlines
	for (unsigned int i = 0; i < numXValues; i++)
	{
		float xPos = plotArea.left() + (xIncrement * i);

		painter.setPen(Qt::darkGray);

		painter.drawLine(QPointF(xPos, plotArea.bottom()), QPointF(xPos, plotArea.top()));

		if (i == 0 || i % dateLabelAlternating == 0)
		{
			const QString& dateStr = m_aAreaChartDates[i];
			float textExtent = getTextExtentHelper(metrics, dateStr);

			painter.setPen(Qt::black);

			yAxisLabelRect.setLeft(xPos - (textExtent / 2.0f));
			// if we're the last label, we're probably going to be off the edge slightly, so push the
			// extent back a bit
			if (i == (numXValues - 1))
			{
				yAxisLabelRect.setLeft(yAxisLabelRect.left() - 10);
			}

			yAxisLabelRect.setRight(xPos + (textExtent / 2.0f));
			painter.drawText(yAxisLabelRect, Qt::AlignCenter, dateStr);
		}
	}

	// draw Y gridlines and labels

	// work out number of y gridlines
	unsigned int numYGrids = 0.4f * std::sqrt((float)plotArea.height());
	int nRem = numYGrids % 2;
	// enforce even numbers so that the scale will look nicer
	numYGrids += nRem;

	float YInc = ceilMaxValue / numYGrids;

	numYGrids++;

	int textHeight = metrics.height();

	for (unsigned int i = 0; i < numYGrids; i++)
	{
		float thisYValue = plotArea.top() + ((YInc * i) * YScale);

		fixed lineValue = (double)ceilMaxValue - (double)(YInc * i);
		QString lineValueString = currencyHandler->formatCurrencyAmount(lineValue);

		painter.setPen(Qt::darkGray);

		painter.drawLine(QPointF(plotArea.left(), thisYValue), QPointF(plotArea.right(), thisYValue));

		painter.setPen(Qt::black);

		painter.drawText(QPoint(20, thisYValue + (textHeight / 2)), lineValueString);
	}

	std::vector<double> baseLineValues(0.0);
	baseLineValues.resize(numXValues);

	unsigned int colourIndex = 0;

	painter.setPen(Qt::black);
	
	m_areaItemPolygons.clear();
	m_areaItemPolygons.resize(m_aAreaChartDataItems.size());
	
	// TODO: try and not generate empty polygons...

	int shapeIndex = 0;
	for (const AreaChartItemValues& item : m_aAreaChartDataItems)
	{
		QPolygonF& itemShape = m_areaItemPolygons[shapeIndex++];

		// draw baseline values anti-clockwise (from left hand side)
		for (unsigned int i = 0; i < numXValues; i++)
		{
			float xPos = plotArea.left() + (xIncrement * i);

			float yVal = baseLineValues[i] * YScale;

			// flip y origin
			yVal = plotArea.height() - yVal;
			yVal += plotArea.top();

			itemShape.append(QPointF(xPos, yVal));
		}

		// then add new values, and add points right-to-left to close the shape

		for (unsigned int i = 0; i < numXValues; i++)
		{
			unsigned int inverseIndex = (numXValues - 1) - i;
			float xPos = plotArea.left() + (xIncrement * inverseIndex);

			// add new data value to baseline value
			baseLineValues[inverseIndex] += item.values[inverseIndex];

			float yVal = baseLineValues[inverseIndex] * YScale;

			// flip y origin
			yVal = plotArea.height() - yVal;
			yVal += plotArea.top();

			itemShape.append(QPointF(xPos, yVal));
		}

		QColor fillColour = m_aAreaColours[colourIndex++];
		if (m_selectedItemIndex == (shapeIndex - 1))
		{
			fillColour.setAlpha(64);
		}
		painter.setBrush(QBrush(fillColour));
		if (colourIndex >= m_aAreaColours.size())
		{
			colourIndex = 0;
		}
		
		painter.drawPolygon(itemShape, Qt::WindingFill);
	}
	
	painter.setPen(Qt::black);
	
	QRect textRect(QPoint(10, 10), QPoint(600, 25));
	
	if (m_selectedItemIndex != -1)
	{
		const AreaChartItemValues& item = m_aAreaChartDataItems[m_selectedItemIndex];
		
		QString selectedItemText = "Selected item: " + QString(item.title.c_str());
		painter.drawText(textRect, selectedItemText);
		
		// configure thick selection line for outline, and no brush for no fill colour
		QPen selectionPen(QColor(192, 192, 192), 2, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin);
		painter.setPen(selectionPen);
		painter.setBrush(Qt::NoBrush);
		
		const QPolygonF& itemShape = m_areaItemPolygons[m_selectedItemIndex];
		painter.drawPolygon(itemShape, Qt::WindingFill);
	}
	else
	{
		painter.drawText(textRect, QString("Selected item: "));
	}
}

void GraphDrawWidget::drawOverviewChart(QPainter& painter, QPaintEvent* event)
{
	if (m_aOverviewChartItems.empty())
		return;

	QFontMetrics metrics(font());
	
	int textHeight = metrics.height();

	UICurrencyHandler* currencyHandler = m_pStashWindow->getCurrencyHandler();
	QString maxValueString = currencyHandler->formatCurrencyAmount(m_maxOverviewValue);

	float leftMargin = 30.0f + getTextExtentHelper(metrics, maxValueString);
	int bottomMargin = (metrics.height() * 2) + 20;
	int topMargin = 40;

	painter.setRenderHint(QPainter::Antialiasing);

	int totalMonthValues = m_aOverviewChartItems.size();

	QRect plotArea = geometry();
	plotArea.adjust(leftMargin, topMargin, -20, -bottomMargin);

	painter.drawRect(plotArea);
	
	// work out how many months we can fit in a screen
	
	int kBarWidth = 28;
	int kGapBetweenMonths = 8;
	int kGapBetweenBars = 2;
	int kItemWidth = (kBarWidth * 2) + kGapBetweenMonths + kGapBetweenBars;
	
	int monthsToShow = plotArea.width() / kItemWidth;
	if (monthsToShow != m_overviewMonthsToShow) // if the window's been resized, reset the page to the first one
	{
		m_overviewCurrentPage = 0;
		m_overviewMonthsToShow = monthsToShow;
	}
	
	if (totalMonthValues > monthsToShow)
	{
		int numPages = totalMonthValues / monthsToShow;
		
		if (totalMonthValues % monthsToShow > 0)
			numPages ++;
		
		m_overviewMultiplePages = true;
		
		// draw scroll arrows at top
		
		int centreX = geometry().width() / 2.0;
		
		painter.setBrush(QBrush(Qt::black));
		
		static const int kArrowSize = 20;
		
		if (m_overviewCurrentPage > 0)
		{
			m_overviewPageControlRegions[0] = QRect(centreX - 120, 10, kArrowSize, kArrowSize);
			QPolygon arrowPoly = createArrowPolylineFromRect(m_overviewPageControlRegions[0], true);
			painter.drawPolygon(arrowPoly);
		}
		else
		{
			// set to empty, so that hit detection for mouse down doesn't work
			m_overviewPageControlRegions[0] = QRect();
		}
		
		if (m_overviewCurrentPage < numPages - 1)
		{
			m_overviewPageControlRegions[1] = QRect(centreX + 120 - kArrowSize, 10, kArrowSize, kArrowSize);
			QPolygon arrowPoly = createArrowPolylineFromRect(m_overviewPageControlRegions[1], false);
			painter.drawPolygon(arrowPoly);
		}
		else
		{
			// set to empty, so that hit detection for mouse down doesn't work
			m_overviewPageControlRegions[1] = QRect();
		}
		
		// draw page circles
		
		int kCircleWidth = 8;
		int kGap = 3;
		
		int circlesFullWidth = numPages * (kCircleWidth + kGap) - kGap;
		
		QRect widgetArea = geometry();
		
		int paginatorYHeight = 16;
		
		int paginatorXStart = widgetArea.width() - (widgetArea.width() / 2.0) - (circlesFullWidth / 2.0);
		
		painter.setPen(Qt::black);
		
		for (int i = 0; i < numPages; i++)
		{
			if (i == m_overviewCurrentPage)
			{
				painter.setBrush(QBrush(Qt::black));
			}
			else
			{
				painter.setBrush(Qt::NoBrush);
			}
			
			painter.drawEllipse(paginatorXStart, paginatorYHeight, kCircleWidth, kCircleWidth);
			
			paginatorXStart += kCircleWidth + kGap;
		}
	}
	
	// work out the max Y value for the items we're showing on screen
	
	if (monthsToShow > totalMonthValues)
		monthsToShow = totalMonthValues;
	
	int startItemIndex = (m_overviewCurrentPage * monthsToShow);
	int endItemIndex = startItemIndex + monthsToShow;
	
	if (endItemIndex > totalMonthValues)
		endItemIndex = totalMonthValues;
	
	fixed maxValue = 0.0;
	
	for (int index = startItemIndex; index < endItemIndex; index++)
	{
		const OverviewChartItem& chartItem = m_aOverviewChartItems[index];
				
		if (chartItem.m_income > maxValue)
			maxValue = chartItem.m_income;
		
		if (chartItem.m_outgoings > maxValue)
			maxValue = chartItem.m_outgoings;
	}
	
	// try and generate a "nice" number which divides easily
	double ceilMaxValue = std::ceil(std::ceil(maxValue.ToDouble() / 10.0) * 10.0);
	
	double YScale = plotArea.height() / ceilMaxValue;
	
	// draw Y gridlines and labels
	
	// work out number of y gridlines
	int numYGrids = 0.4 * std::sqrt((double)plotArea.height());
	int nRem = numYGrids % 2;
	// enforce even numbers so that the scale will look nicer
	numYGrids += nRem;
	
	double YInc = ceilMaxValue / numYGrids;
	numYGrids++;
	
	for (int i = 0; i < numYGrids; i++)
	{		
		float thisYValue = plotArea.top() + ((YInc * i) * YScale);

		fixed lineValue = (double)ceilMaxValue - (double)(YInc * i);
		QString lineValueString = currencyHandler->formatCurrencyAmount(lineValue);

		painter.setPen(Qt::darkGray);

		painter.drawLine(QPointF(plotArea.left(), thisYValue), QPointF(plotArea.right(), thisYValue));

		painter.setPen(Qt::black);

		painter.drawText(QPoint(20, thisYValue + (textHeight / 2)), lineValueString);
	}
	
	// draw the bars
	
	QRect yAxisLabelRect(0, plotArea.bottom() + 10, 40, 30);
	
	int plotIndex = 0;
	for (int index = startItemIndex; index < endItemIndex; index++, plotIndex++)
	{
		const OverviewChartItem& chartItem = m_aOverviewChartItems[index];
		
		double overallXStart = plotArea.left() + (plotIndex * kItemWidth);
		double dXStart = overallXStart + (kGapBetweenMonths / 2);
		
		painter.setBrush(QBrush(Qt::blue));
		
		double incomeBarHeight = chartItem.m_income.ToDouble() * YScale;
		// flip y origin
		double incomeBarYTop = plotArea.height() + plotArea.top() - incomeBarHeight;
		
		// don't want anti-aliasing for the bars...
		painter.setRenderHint(QPainter::Antialiasing, false);
		
		painter.drawRect(dXStart, incomeBarYTop, kBarWidth, incomeBarHeight);
		
		painter.setBrush(QBrush(Qt::red));
		
		double outgoingsBarHeight = chartItem.m_outgoings.ToDouble() * YScale;
		// flip y origin
		double outgoingsBarYTop = plotArea.height() + plotArea.top() - outgoingsBarHeight;
		
		painter.drawRect(dXStart + kBarWidth + kGapBetweenBars, outgoingsBarYTop, kBarWidth, outgoingsBarHeight);
		
		// draw date text at the bottom
		
		const QString& dateStr = m_aOverviewChartDates[index];
//		float textExtent = getTextExtentHelper(metrics, dateStr);

//		yAxisLabelRect.setLeft(xPos - (textExtent / 2.0f));
		yAxisLabelRect.setLeft(overallXStart);

//		yAxisLabelRect.setRight(xPos + (textExtent / 2.0f));
		yAxisLabelRect.setRight(overallXStart + kItemWidth);
		
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.drawText(yAxisLabelRect, Qt::AlignCenter, dateStr);
	}
	
	painter.setRenderHint(QPainter::Antialiasing);
}

void GraphDrawWidget::displayPopupMenu(const QPoint& pos)
{
	if (m_selectedItemIndex == -1)
		return;
	
	if (m_graphType == eGraphOverviewChart)
		return;
	
	QMenu menu(this);
	
	menu.addAction(m_pMenuAddSelectedItem);
	
	menu.exec(mapToGlobal(pos));
}

QPolygon GraphDrawWidget::createArrowPolylineFromRect(const QRect& rectBounds, bool pointLeft)
{
	QPolygon finalPolygon;
	
	if (pointLeft)
	{
		finalPolygon.append(QPoint(rectBounds.left(), rectBounds.top() + (rectBounds.height() / 2)));
		finalPolygon.append(QPoint(rectBounds.right(), rectBounds.top()));
		finalPolygon.append(QPoint(rectBounds.right(), rectBounds.bottom()));
	}
	else
	{
		finalPolygon.append(QPoint(rectBounds.right(), rectBounds.top() + (rectBounds.height() / 2)));
		finalPolygon.append(QPoint(rectBounds.left(), rectBounds.bottom()));
		finalPolygon.append(QPoint(rectBounds.left(), rectBounds.top()));
	}
	
	return finalPolygon;
}

int GraphDrawWidget::getTextExtentHelper(const QFontMetrics& fontMetrics, const QString& stringValue)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	return fontMetrics.horizontalAdvance(stringValue);
#else
	return fontMetrics.width(stringValue);
#endif
}
