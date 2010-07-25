/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2010 Peter Pearson
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

#import "OverviewChartView.h"

@implementation OverviewChartView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
        m_aData = [[NSMutableArray alloc] init];
		
		m_nCurrentPage = 0;
    }
    return self;
}

- (void)dealloc
{
	[m_aData release];
	
	[super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect bounds = [self bounds];
	[[NSColor whiteColor] set];
	[NSBezierPath fillRect:bounds];
	[[NSColor blackColor] set];
	[NSBezierPath strokeRect:bounds];
	
	NSMutableDictionary *attributes1 = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Helvetica" size:12], NSFontAttributeName, nil];
	
	int nNumXValues = [m_aData count];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLenient:YES];
	
	NSNumber *nMaxAmount = [NSNumber numberWithDouble:m_dMaxValue];
	NSString *sMaxAmount = [numberFormatter stringFromNumber:nMaxAmount];
	NSSize maxAmountSize = [sMaxAmount sizeWithAttributes:attributes1];	
	
	double dMarginX = maxAmountSize.width + 30;
	double dMarginY = 40;
	
	NSRect innerBounds = NSInsetRect(bounds, (dMarginX / 2.0), dMarginY);
	NSRect plotArea = NSOffsetRect(innerBounds, (dMarginX / 3.5), 0);
	
	[[NSColor blackColor] set];	
	[NSBezierPath strokeRect:plotArea];
	
	// work out how many months we can fit in a screen
	
	int nBarWidth = 28;
	int nGapBetweenMonths = 8;
	int nGapBetweenBars = 2;
	int nItemWidth = (nBarWidth * 2) + nGapBetweenMonths + nGapBetweenBars;
	
	int nMonthsToShow = plotArea.size.width / nItemWidth;
	if (nMonthsToShow != m_nMonthsToShow) // if the window's been resized, reset the page to the first one
	{
		m_nCurrentPage = 0;
		m_nMonthsToShow = nMonthsToShow;
	}
	
	if (nNumXValues > nMonthsToShow)
	{
		int nPages = nNumXValues / nMonthsToShow;
		
		if (nNumXValues % nMonthsToShow > 0)
			nPages ++;
		
		m_bMultiplePages = true;
		
		NSImage *pImage = 0;
		
		if (m_nCurrentPage > 0)
		{
			pImage = [NSImage imageNamed:NSImageNameGoLeftTemplate];
			[pImage setSize:NSMakeSize(16, 16)];
			m_leftArrow = NSMakeRect(16, bounds.size.height - 24, 16, 16);
			[pImage compositeToPoint:m_leftArrow.origin operation:NSCompositeSourceOver];		
		}
		
		if (m_nCurrentPage < (nPages - 1))
		{		
			pImage = [NSImage imageNamed:NSImageNameGoRightTemplate];
			[pImage setSize:NSMakeSize(16, 16)];
			m_rightArrow = NSMakeRect(bounds.size.width - 16 - 16, bounds.size.height - 24, 16, 16);
			[pImage compositeToPoint:m_rightArrow.origin operation:NSCompositeSourceOver];
		}
		
		// draw page circles at top
		
		int circleWidth = 8;
		int gap = 3;
		
		int nFullWidth = nPages * (circleWidth + gap) - gap;
		
		double dYHeight = bounds.size.height - 8 - circleWidth;
		
		double dXStart = bounds.size.width - (bounds.size.width / 2.0) - (nFullWidth / 2.0);
		
		for (int i = 0; i < nPages; i++)
		{
			NSRect circleRect = NSMakeRect(dXStart, dYHeight, circleWidth, circleWidth);
			NSBezierPath* circlePath = [NSBezierPath bezierPath];
			[circlePath appendBezierPathWithOvalInRect:circleRect];
			
			[circlePath stroke];
			
			if (i == m_nCurrentPage)
			{
				[circlePath fill];
			}
			
			dXStart += circleWidth + gap;			
		}		
	}
	
	double dLeftStart = plotArea.origin.x;
	double dBottomStart = plotArea.origin.y;
	
	// work out the max Y value for the items we're showing on screen
	
	if (nMonthsToShow > nNumXValues)
		nMonthsToShow = nNumXValues;
	
	int nStartItem = (m_nCurrentPage * nMonthsToShow);
	int nMaxItem = nStartItem + nMonthsToShow;
	
	if (nMaxItem > nNumXValues)
		nMaxItem = nNumXValues;
	
	double dMaxValue = 0.0;
	
	for (int index = nStartItem; index < nMaxItem; index++)
	{
		NSMutableDictionary *plotItem = [m_aData objectAtIndex:index];
		double dIncome = [[plotItem objectForKey:@"income"] doubleValue];
		double dOutgoings = [[plotItem objectForKey:@"outgoings"] doubleValue];
		
		if (dIncome > dMaxValue)
			dMaxValue = dIncome;
		
		if (dOutgoings > dMaxValue)
			dMaxValue = dOutgoings;		
	}
	
	// try and generate a "nice" number which divides easily
	double dCeilMaxValue = ceil(ceil(dMaxValue / 10.0) * 10.0);
	
	double dYScale = plotArea.size.height / dCeilMaxValue;
	
	NSBezierPath *gridLine = [NSBezierPath bezierPath];
	
	// draw Y gridlines and labels
	
	// work out number of y gridlines
	int numYGrids = 0.4 * sqrt(plotArea.size.height);
	int nRem = numYGrids % 2;
	// enforce even numbers so that the scale will look nicer
	numYGrids += nRem;
	
	double dYInc = dCeilMaxValue / numYGrids;
	
	numYGrids++;
	
	for (int i = 0; i < numYGrids; i++)
	{
		double dYValue = dBottomStart + (dYInc * i) * dYScale;
		
		[gridLine moveToPoint:NSMakePoint(plotArea.origin.x, dYValue)];
		[gridLine lineToPoint:NSMakePoint(plotArea.origin.x + plotArea.size.width, dYValue)];
		
		NSNumber *nScaleAmount = [NSNumber numberWithDouble:(dYInc * i)];
		NSString *sScaleAmount = [[numberFormatter stringFromNumber:nScaleAmount] retain];
		
		NSSize extent = [sScaleAmount sizeWithAttributes:attributes1];
		
		[sScaleAmount drawAtPoint:NSMakePoint(plotArea.origin.x - extent.width - 3, dYValue - (extent.height / 2.0)) withAttributes:attributes1];				
	}
	
	[numberFormatter release];
	
	[gridLine setLineWidth:1.0];
	[[NSColor grayColor] set];
	[gridLine stroke];
	
	int nPlot = 0;
	
	for (int index = nStartItem; index < nMaxItem; index++, nPlot++)
	{
		NSMutableDictionary *plotItem = [m_aData objectAtIndex:index];
		double dIncome = [[plotItem objectForKey:@"income"] doubleValue];
		double dOutgoings = [[plotItem objectForKey:@"outgoings"] doubleValue];
		
		double dXStart = dLeftStart + (nPlot * nItemWidth) + (nGapBetweenMonths / 2);
		
		[[NSGraphicsContext currentContext] setShouldAntialias:NO];
		
		NSRect incomeBar = NSMakeRect(dXStart, dBottomStart, nBarWidth, dIncome * dYScale);
		[[NSColor blueColor] set];
		NSRectFill(incomeBar);
		[[NSColor blackColor] set];
		[NSBezierPath setDefaultLineWidth:1.0];
		[NSBezierPath strokeRect:incomeBar];
		
		NSRect outgoingsBar = NSMakeRect(dXStart + nBarWidth + nGapBetweenBars, dBottomStart, nBarWidth, dOutgoings * dYScale);
		[[NSColor redColor] set];
		NSRectFill(outgoingsBar);
		[[NSColor blackColor] set];
		[NSBezierPath setDefaultLineWidth:1.0];
		[NSBezierPath strokeRect:outgoingsBar];
		
		[[NSGraphicsContext currentContext] setShouldAntialias:YES];
		
		NSString *sDate = [plotItem valueForKey:@"date"];
		NSSize extent = [sDate sizeWithAttributes:attributes1];
		
		[sDate drawAtPoint:NSMakePoint(dXStart + (nItemWidth / 2) - (extent.width / 2.0), 5) withAttributes:attributes1];
	}	
}

- (void)setData:(NSMutableArray*)data
{
	[m_aData removeAllObjects];
	[m_aData addObjectsFromArray:data];
	
	[self setNeedsDisplay:YES];
}

- (void)setMaxValue:(double)value
{
	m_dMaxValue = value;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if (m_bMultiplePages)
	{
		NSPoint eventLocation = [theEvent locationInWindow];
		NSPoint newPoint = [self convertPoint:eventLocation fromView:nil];
		
		if (NSPointInRect(newPoint, m_leftArrow))
		{
			if (m_nCurrentPage > 0)
			{
				m_nCurrentPage--;
				
				[self setNeedsDisplay:YES];
			}		
		}
		else if (NSPointInRect(newPoint, m_rightArrow))
		{
			m_nCurrentPage++;
			
			[self setNeedsDisplay:YES];
		}
	}	
}

@end
