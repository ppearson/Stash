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

#include "fixed.h"
#import "PieChartView.h"
#import "GraphController.h"
#import "ValueFormatter.h"

#define DegToRad(deg) (deg*0.017453)
#define RadToDeg(deg) (deg*57.2958)

@implementation PieChartView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
        m_aData = [[NSMutableArray alloc] init];
		m_aColours = [[NSMutableArray alloc] init];
		
		[m_aColours addObject:[NSColor grayColor]];
		[m_aColours addObject:[NSColor redColor]];
		[m_aColours addObject:[NSColor yellowColor]];
		[m_aColours addObject:[NSColor greenColor]];
		[m_aColours addObject:[NSColor blueColor]];
		[m_aColours addObject:[NSColor orangeColor]];
		[m_aColours addObject:[NSColor brownColor]];
		
		m_mouseDownSegment = -1;
    }
    return self;
}

- (void)dealloc
{
	[m_aData release];
	[m_aColours release];
	
	[super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect bounds = [self bounds];
	[[NSColor whiteColor] set];
	[NSBezierPath fillRect:bounds];
	[[NSColor blackColor] set];
	[NSBezierPath strokeRect:bounds];
	
	double dSmallestSize = 0.0;
	
	if (bounds.size.height < bounds.size.width)
		dSmallestSize = bounds.size.height;
	else
		dSmallestSize = bounds.size.width;
	
	m_dMainRadius = (dSmallestSize / 2.0) - 25.0;
	
	m_centrepoint = NSMakePoint(bounds.size.width / 2.0, bounds.size.height / 2.0);
	
	NSBezierPath* path = [NSBezierPath bezierPath];
	
	int selectionType = [[NSUserDefaults standardUserDefaults] integerForKey:@"PieChartSelectionType"];
	
	BOOL bPushoutSelected = NO;
	
	if (selectionType == 1)
		bPushoutSelected = YES;
	
	id colour;
	int nColourIndex = 0;
	
	fixed selectedAmount = 0.0;
	
	NSMutableDictionary *item = 0;
	
	NSMutableDictionary *attributes1 = [NSMutableDictionary dictionary];
	[attributes1 setObject:[NSFont fontWithName:@"Helvetica" size:12] forKey:NSFontAttributeName];
		
	for (item in m_aData)
	{
		double dStartAngle = [[item valueForKey:@"startangle"] doubleValue];
		double dEndAngle = [[item valueForKey:@"endangle"] doubleValue];
		
		BOOL bSelected = [[item valueForKey:@"selected"] boolValue];
		
		double dMidPointAngle = ((dEndAngle - dStartAngle) / 2.0) + dStartAngle;
		
		double dRadius = m_dMainRadius;
		
		if (bPushoutSelected && bSelected)
		{
			dRadius += 10.0;
		}
						
		[path moveToPoint:m_centrepoint];
		
		[path appendBezierPathWithArcWithCenter:m_centrepoint radius:dRadius startAngle:dStartAngle endAngle:dEndAngle clockwise:NO];
		[path lineToPoint:m_centrepoint];
		
		colour = [[m_aColours objectAtIndex:nColourIndex] colorWithAlphaComponent:0.8f];
		
		[(NSColor*)colour set];
		[path fill];
		
		// draw gloss gradient
		NSGradient* aGradient = [[[NSGradient alloc]
								  initWithColorsAndLocations:[[NSColor grayColor] colorWithAlphaComponent:0.35f], (CGFloat)0.0,
								  [[NSColor whiteColor] colorWithAlphaComponent:0.3f], (CGFloat)1.0, nil] autorelease];
		
		[aGradient drawInBezierPath:path angle:90.0];
		
		[path setLineWidth:1.0];
		[[NSColor grayColor] set];
			
		[path stroke];
				
		[path removeAllPoints];

		nColourIndex ++;
		
		if (nColourIndex > 6)
			nColourIndex = 1;
	}
	
	// do the same loop again in order to draw selected segments and the labels on top
	for (item in m_aData)
	{
		double dStartAngle = [[item valueForKey:@"startangle"] doubleValue];
		double dEndAngle = [[item valueForKey:@"endangle"] doubleValue];
		
		BOOL bSelected = [[item valueForKey:@"selected"] boolValue];
		
		// draw text label		
		double dMidPointAngle = ((dEndAngle - dStartAngle) / 2.0) + dStartAngle;
		
		double dRad = DegToRad(dMidPointAngle);
		double dX, dY = 0.0;
		
		dY = sin(dRad);
		dX = cos(dRad);
		
		if (bSelected == YES)
		{
			NSNumber *sliceAmount = [item valueForKey:@"numamount"];
			
			selectedAmount += [sliceAmount doubleValue];
			
			if (!bPushoutSelected)
			{
				[NSGraphicsContext saveGraphicsState];
				
				NSSetFocusRingStyle(NSFocusRingOnly);
				
				[path moveToPoint:m_centrepoint];
				
				[path appendBezierPathWithArcWithCenter:m_centrepoint radius:m_dMainRadius startAngle:dStartAngle endAngle:dEndAngle clockwise:NO];
				[path lineToPoint:m_centrepoint];
				
				[path setLineWidth:3.0];
				
				[path stroke];
				
				[NSGraphicsContext restoreGraphicsState];
				
				[path removeAllPoints];
			}
		}		
		
		NSString *title = [item valueForKey:@"title"];
		NSString *amount = [item valueForKey:@"amount"];
		
		NSString *strText = [NSString stringWithFormat:@"%@  %@", title, amount];
		NSSize extent = [strText sizeWithAttributes:attributes1];
		
		double dStartX = m_centrepoint.x + (dX * m_dMainRadius);
		double dStartY = m_centrepoint.y + (dY * m_dMainRadius);
		
		dStartX -= (extent.width / 2.0);
		dStartY -= (extent.height / 2.0);
		
		// check to see if we're going to fall off the end - if so, move the text
		// left a bit
		
		if ((dStartX + extent.width) >= bounds.size.width)
		{
			dStartX -= (dStartX + extent.width) - bounds.size.width;
		}
		else if (dStartX < bounds.origin.x)
		{
			dStartX = 0.0;
		}
		
		NSPoint labelPosition = NSMakePoint(dStartX, dStartY);		
		
		[strText drawAtPoint:labelPosition withAttributes:attributes1];
	}
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	NSString *sSelAmount = [[valueFormatter currencyStringFromFixed:selectedAmount] retain];
	
	if (m_total)
	{
		NSString *strText = [NSString stringWithFormat:NSLocalizedString(@"Total amount:  %@", "Pie Chart View -> Total Amount Text"), m_total];
		[strText drawAtPoint:NSMakePoint(5.0, 5.0) withAttributes:attributes1];
		
		strText = [NSString stringWithFormat:NSLocalizedString(@"Selected amount:  %@", "Pie Chart View -> Selected Amount Text"), sSelAmount];
		[strText drawAtPoint:NSMakePoint(5.0, 25.0) withAttributes:attributes1];
	}
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint eventLocation = [theEvent locationInWindow];
    NSPoint newPoint = [self convertPoint:eventLocation fromView:nil];
	
	NSMutableDictionary *slice = 0;
	
	double dXDiff = newPoint.x - m_centrepoint.x;
	double dYDiff = newPoint.y - m_centrepoint.y;
	
	double dAngle = atan2(dYDiff, dXDiff);
	
	double dAngle2 = RadToDeg(dAngle);
	
	if (dAngle2 < 0.0)
		dAngle2 += 360.0;
	
	double dDistanceOut = sqrt(dYDiff * dYDiff + dXDiff * dXDiff);
	
	// if Command key not held down to perform multiple selection, deselect all segments
	if (!([theEvent modifierFlags] & NSCommandKeyMask))
	{
		for (slice in m_aData)
		{
			[slice setValue:[NSNumber numberWithBool:NO] forKey:@"selected"];
		}
	}
	
	m_mouseDownSegment = -1;
	
	int segment = 0;
	for (slice in m_aData)
	{
		double dStartAngle = [[slice valueForKey:@"startangle"] doubleValue];
		double dEndAngle = [[slice valueForKey:@"endangle"] doubleValue];
		
		if (dAngle2 < dEndAngle && dAngle2 > dStartAngle && dDistanceOut < m_dMainRadius)
		{
			BOOL bSelected = [[slice valueForKey:@"selected"] boolValue];
			
			bSelected = !bSelected;
			
			[slice setValue:[NSNumber numberWithBool:bSelected] forKey:@"selected"];
			
			m_mouseDownSegment = segment;
			
			break;
			
		}
		segment++;
	}
	
	[self setNeedsDisplay:YES];	
}

- (void)setData:(NSMutableArray*)data
{
	[m_aData removeAllObjects];
	
	[m_aData addObjectsFromArray:data];
	
	[self setNeedsDisplay:YES];
}

- (void)setTotal:(NSString*)total
{
	[total retain];
	
	m_total = total;
}

- (BOOL)isOpaque
{
	return YES;
}

- (NSMenu*)menuForEvent:(NSEvent*)event
{
    if (m_mouseDownSegment != -1)
		return fMenu;
	
	return nil;
}

- (IBAction)addSelectedItem:(id)sender
{
	if (m_mouseDownSegment == -1)
		return;
	
	NSMutableDictionary *item = [m_aData objectAtIndex:m_mouseDownSegment];
	
	NSString *sTitle = [item objectForKey:@"title"];
	
	GraphController* pGC = [GraphController sharedInterface];
	[pGC addSelectedGraphItem:sTitle];
	
	m_mouseDownSegment = -1;
}

@end
