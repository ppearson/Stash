/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009 Peter Pearson
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

#import "AreaChartView.h"
#import "GraphController.h"
#import "ValueFormatter.h"

@implementation AreaChartView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
        m_aData = [[NSMutableArray alloc] init];
		m_aColours = [[NSMutableArray alloc] init];
		
		m_aDates = [[NSMutableArray alloc] init];
		m_aPlotShapes = [[NSMutableArray alloc] init];
		
		m_selectedPlot = -1;
		
		[m_aColours addObject:[NSColor blueColor]];
		[m_aColours addObject:[NSColor redColor]];
		[m_aColours addObject:[NSColor yellowColor]];
		[m_aColours addObject:[NSColor greenColor]];
		[m_aColours addObject:[NSColor grayColor]];
		[m_aColours addObject:[NSColor orangeColor]];
		[m_aColours addObject:[NSColor brownColor]];
    }
    return self;
}

- (void)dealloc
{
	[m_aData release];
	[m_aColours release];
	
	[m_aDates release];
	[m_aPlotShapes release];
	
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
	
	bool bProblem = false;
	
	NSMutableDictionary *plotItem = 0;
	NSMutableArray *itemValues = 0;
	int nNumXValues = 0;
	
	if ([m_aData count] > 0)
	{
		plotItem = [m_aData objectAtIndex:0];
		itemValues = [plotItem objectForKey:@"amounts"];
		
		nNumXValues = [itemValues count];
		
		if (nNumXValues > 36)
			nNumXValues = 36;
		
		if (nNumXValues < 2)
		{
			bProblem = true;
		}
	}
	else
	{
		bProblem = true;
	}
	
	if (bProblem)
	{
		NSString *sMessage = NSLocalizedString(@"Not enough data to plot an area chart.\nArea charts are plotted on a monthly basis, and the data range must span over two months.", "Area Chart View -> No Data Message");
		
		NSSize extent = [sMessage sizeWithAttributes:attributes1];
		
		[sMessage drawAtPoint:NSMakePoint(bounds.size.width / 2.0 - (extent.width / 2.0), bounds.size.height / 2.0) withAttributes:attributes1];
		return;
	}
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	NSNumber *nMaxAmount = [NSNumber numberWithDouble:m_dMaxValue];
	NSString *sMaxAmount = [valueFormatter currencyStringFromNumber:nMaxAmount];
	NSSize maxAmountSize = [sMaxAmount sizeWithAttributes:attributes1];	
	
	double dMarginX = maxAmountSize.width + 30;
	double dMarginY = 40;
	
	NSRect innerBounds = NSInsetRect(bounds, (dMarginX / 2.0), dMarginY);
	NSRect plotArea = NSOffsetRect(innerBounds, (dMarginX / 3.5), 0);
	
	[[NSColor blackColor] set];	
	[NSBezierPath strokeRect:plotArea];
	
	double dLeftStart = plotArea.origin.x;
	double dBottomStart = plotArea.origin.y;
	
	// try and generate a "nice" number which divides easily
	double dCeilMaxValue = ceil(ceil(m_dMaxValue / 10.0) * 10.0);
	
	double dXIncrement = plotArea.size.width / (nNumXValues - 1);
	double dYScale = plotArea.size.height / dCeilMaxValue;
	
	double aBaseItems[36];
	
	for (int i = 0; i < 36; i++)
		aBaseItems[i] = 0.0;
	
	int dateLabelAlternating = 1;
	
	// work out how many X-axis labels can fit into the space available
	if (m_sLongestDate)
	{
		NSSize extent = [m_sLongestDate sizeWithAttributes:attributes1];
		// slight margin so that we ensure that labels don't touch
		extent.width += 4.0;
		
		if (extent.width >= dXIncrement)
			dateLabelAlternating = 2;
		
		if (extent.width >= dXIncrement * 2)
			dateLabelAlternating = 3;
	}
	
	// draw X gridlines and labels
	
	NSBezierPath *gridLine = [NSBezierPath bezierPath];
	NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
	[style setAlignment:NSCenterTextAlignment];
	NSDictionary *attr = [NSDictionary dictionaryWithObject:style forKey:NSParagraphStyleAttributeName];
	
	for (int i = 0; i < nNumXValues; i++)
	{
		double dGridX = dLeftStart + (i * dXIncrement);
		
		[gridLine moveToPoint:NSMakePoint(dGridX, dBottomStart)];
		[gridLine lineToPoint:NSMakePoint(dGridX, dBottomStart + plotArea.size.height)];
		
		if (i == 0 || i % dateLabelAlternating == 0)
		{
			NSString *sDate = [m_aDates objectAtIndex:i];
			NSSize extent = [sDate sizeWithAttributes:attributes1];
			
			// if we're the last label, we're probably going to be off the edge slightly, so push the
			// extent back a bit
			if (i == (nNumXValues - 1))
				extent.width += 15.0;
		
			[sDate drawInRect:NSMakeRect(dGridX - (extent.width / 2.0), 5, extent.width, 30) withAttributes:attr];
		}
	}
	
	[style release];
	
	[gridLine setLineWidth:1.0];
	[[NSColor grayColor] set];
	[gridLine stroke];
	
	[gridLine removeAllPoints];
	
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
		NSString *sScaleAmount = [[valueFormatter currencyStringFromNumber:nScaleAmount] retain];
		
		NSSize extent = [sScaleAmount sizeWithAttributes:attributes1];
		
		[sScaleAmount drawAtPoint:NSMakePoint(plotArea.origin.x - extent.width - 3, dYValue - (extent.height / 2.0)) withAttributes:attributes1];				
	}
	
	[gridLine setLineWidth:1.0];
	[[NSColor grayColor] set];
	[gridLine stroke];
	
	NSBezierPath *plotShape = 0;
	NSBezierPath *separatorLine = [NSBezierPath bezierPath];
	
	int nColourIndex = 0;
	id colour;
	
	[m_aPlotShapes removeAllObjects];
	
	int nPlot = 0;
	
	for (plotItem in m_aData)
	{
		plotShape = [NSBezierPath bezierPath];
		itemValues = [plotItem objectForKey:@"amounts"];
		
		NSNumber *amount = 0;
		
		double dYValue = aBaseItems[0] * dYScale;
		
		double dYStart = dBottomStart + dYValue;
		double dXStart = dLeftStart + (0 * dXIncrement);
		
		[plotShape moveToPoint:NSMakePoint(dXStart, dYStart)];
		
		// Draw baseline amounts		
		for (int nAmountIndex = 1; nAmountIndex < nNumXValues; nAmountIndex++)
		{
			amount = [itemValues objectAtIndex:nAmountIndex];
			
			dYValue = aBaseItems[nAmountIndex] * dYScale;
			
			[plotShape lineToPoint:NSMakePoint(dLeftStart + (nAmountIndex * dXIncrement), dBottomStart + dYValue)];			
		}
		
		amount = [itemValues objectAtIndex:nNumXValues - 1];
		
		aBaseItems[nNumXValues - 1] += [amount doubleValue];
		
		dYValue = aBaseItems[nNumXValues - 1] * dYScale;
		
		[plotShape lineToPoint:NSMakePoint(dLeftStart + ((nNumXValues - 1) * dXIncrement), dBottomStart + dYValue)];
		
		[separatorLine moveToPoint:NSMakePoint(dLeftStart + ((nNumXValues - 1) * dXIncrement), dBottomStart + dYValue)];
										  
		for (int nAmountIndex = nNumXValues - 2; nAmountIndex >= 0; nAmountIndex--)
		{
			amount = [itemValues objectAtIndex:nAmountIndex];
			
			aBaseItems[nAmountIndex] += [amount doubleValue];
		  
			dYValue = aBaseItems[nAmountIndex] * dYScale;
		  
			[plotShape lineToPoint:NSMakePoint(dLeftStart + (nAmountIndex * dXIncrement), dBottomStart + dYValue)];
			
			[separatorLine lineToPoint:NSMakePoint(dLeftStart + (nAmountIndex * dXIncrement), dBottomStart + dYValue)];
		}
		
		[plotShape lineToPoint:NSMakePoint(dXStart, dYStart)];
		
		if (m_selectedPlot == nPlot)
		{
			[[NSColor grayColor] set];
			CGFloat pattern[2] = {4.0, 4.0};
			
			[plotShape setLineDash:pattern count:2 phase:0.0];
			[plotShape setLineWidth:3.0];
			[plotShape stroke];
			
			[plotShape setLineDash:NULL count:0 phase:0.0];
			
			colour = [[m_aColours objectAtIndex:nColourIndex] colorWithAlphaComponent:0.2f];
			
			// draw selected item title
			NSString *sSelectedItem = [NSString stringWithFormat:NSLocalizedString(@"Selected item: %@", "Area Chart View -> Selected Item Text"),
									   [plotItem objectForKey:@"title"]];
			NSSize extent = [sSelectedItem sizeWithAttributes:attributes1];
			
			[sSelectedItem drawAtPoint:NSMakePoint(5, bounds.size.height - 5 - extent.height) withAttributes:attributes1];
		}
		else
		{
			colour = [[m_aColours objectAtIndex:nColourIndex] colorWithAlphaComponent:0.5f];
			
			[plotShape setLineWidth:1.0];
			[[NSColor blackColor] set];
			
			[separatorLine stroke];			
		}
		
		[(NSColor*)colour set];
		[plotShape fill];
		
		[m_aPlotShapes addObject:plotShape];
		[separatorLine removeAllPoints];
		
		nColourIndex ++;
		
		if (nColourIndex > 6)
			nColourIndex = 0;
		
		nPlot ++;
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

- (void)setDates:(NSMutableArray*)dates
{
	[m_aDates removeAllObjects];
	
	[m_aDates addObjectsFromArray:dates];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint eventLocation = [theEvent locationInWindow];
    NSPoint newPoint = [self convertPoint:eventLocation fromView:nil];
	
	NSBezierPath *plotShape = 0;
	
	int nPlot = 0;
	
	for (plotShape in m_aPlotShapes)
	{
		if ([plotShape containsPoint:newPoint])
		{
			m_selectedPlot = nPlot;
			
			[self setNeedsDisplay:YES];	
			
			return;			
		}		
		
		nPlot ++;
	}
	
	m_selectedPlot = -1;
	
	[self setNeedsDisplay:YES];
}

- (NSMenu*)menuForEvent:(NSEvent*)event
{
    if (m_selectedPlot != -1)
		return fMenu;
	
	return nil;
}

- (IBAction)addSelectedItem:(id)sender
{
	if (m_selectedPlot == -1)
		return;
	
	NSMutableDictionary *item = [m_aData objectAtIndex:m_selectedPlot];
	
	NSString *sTitle = [item objectForKey:@"title"];
	
	GraphController* pGC = [GraphController sharedInterface];
	[pGC addSelectedGraphItem:sTitle];
	
	m_selectedPlot = -1;
}
		
- (void)setLongestDate:(NSString*)longestDate
{
	[m_sLongestDate retain];
	m_sLongestDate = longestDate;
}

@end
