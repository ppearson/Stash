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

#import "PieChartView.h"

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
		
		[m_aColours addObject:[NSColor blueColor]];
		[m_aColours addObject:[NSColor redColor]];
		[m_aColours addObject:[NSColor yellowColor]];
		[m_aColours addObject:[NSColor greenColor]];
		[m_aColours addObject:[NSColor grayColor]];
    }
    return self;
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
	
	double dMainRadius = (dSmallestSize / 2.0) - 25.0;
	
	NSPoint centrepoint = NSMakePoint(bounds.size.width / 2.0, bounds.size.height / 2.0);
	
	NSBezierPath* path = [NSBezierPath bezierPath];
	
	id colour;
	int nColourIndex = 0;
	
	double dStartAngle = 0.0;
	double dEndAngle = 0.0;
	
	NSMutableDictionary *item = 0;
	
	NSMutableDictionary *attributes1 = [NSMutableDictionary dictionary];
	[attributes1 setObject:[NSFont fontWithName:@"Helvetica" size:12] forKey:NSFontAttributeName];
	
	for (item in m_aData)
	{
		double dAngle = [[item valueForKey:@"angle"] doubleValue];
		
		dEndAngle += dAngle;
		
		[path moveToPoint:centrepoint];
		
		[path appendBezierPathWithArcWithCenter:centrepoint radius:dMainRadius startAngle:dStartAngle endAngle:dEndAngle clockwise:NO];
		[path lineToPoint:centrepoint];
		
		colour = [[m_aColours objectAtIndex:nColourIndex] colorWithAlphaComponent:0.5f];
		
		[colour set];
		[path fill];		
		
		[[NSColor grayColor] set];
		[path stroke];
		
		[path removeAllPoints];

		dStartAngle += dAngle;
		
		nColourIndex ++;
		
		if (nColourIndex > 4)
			nColourIndex = 0;
	}
	
	// do the same loop again in order to draw the labels on top
	
	dStartAngle = 0.0;
	dEndAngle = 0.0;
	
	for (item in m_aData)
	{
		double dAngle = [[item valueForKey:@"angle"] doubleValue];
		NSString *title = [item valueForKey:@"title"];
		NSString *amount = [item valueForKey:@"amount"];
		
		dEndAngle += dAngle;
		
		// draw text label		
		double dMidPointAngle = ((dEndAngle - dStartAngle) / 2.0) + dStartAngle;
		
		double dRad = DegToRad(dMidPointAngle);
		double dX, dY = 0.0;
		
		dY = sin(dRad);
		dX = cos(dRad);
		
		NSString *strText = [NSString stringWithFormat:@"%@  %@", title, amount];
		NSSize extent = [strText sizeWithAttributes:attributes1];
		
		double dStartX = centrepoint.x + (dX * dMainRadius);
		double dStartY = centrepoint.y + (dY * dMainRadius);
		
		dStartX -= (extent.width / 2.0);
		dStartY -= (extent.height / 2.0);
		
		NSPoint labelPosition = NSMakePoint(dStartX, dStartY);		
		
		[strText drawAtPoint:labelPosition withAttributes:attributes1];
		
		dStartAngle += dAngle;	
	}
	
	
	if (m_total)
	{
		NSString *strText = [NSString stringWithFormat:@"Total amount:  %@", m_total];
		[strText drawAtPoint:NSMakePoint(5.0, 5.0) withAttributes:attributes1];
	}	
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

@end
