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

#import "IndexBarTextCell.h"

@implementation IndexBarTextCell

- (id)init
{
	if ((self = [super init]) != nil)
	{
		m_amount = nil;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	IndexBarTextCell *cell = (IndexBarTextCell *)[super copyWithZone:zone];

	[cell setAmount:m_amount];
	
	return cell;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{	
	if (m_amount)
	{
		NSFont* font = [super font];
		NSFont* drawFont = font;
		
		if ([NSFont respondsToSelector:@selector(monospacedDigitSystemFontOfSize:weight:)])
		{
			drawFont = [NSFont monospacedDigitSystemFontOfSize:[NSFont smallSystemFontSize] weight:NSFontWeightRegular];
		}
		
		NSShadow *textShadow = [[[NSShadow alloc] init] autorelease];
		[textShadow setShadowColor:[[NSColor blackColor] colorWithAlphaComponent:0.5]];
		[textShadow setShadowOffset:NSMakeSize(1.0, -1.5)];
		[textShadow setShadowBlurRadius:1.0];
		
		BOOL highlighted = [self isHighlighted];
		
		NSDictionary *attrs = [NSDictionary dictionaryWithObjectsAndKeys:
							  drawFont, NSFontAttributeName,
							   highlighted ? [NSColor whiteColor] : [NSColor darkGrayColor], NSForegroundColorAttributeName,
							   highlighted ? textShadow : nil, highlighted ? NSShadowAttributeName : nil,
							  nil, nil];
		
		NSRect textRect = cellFrame;
		
		NSSize cellSize = [super cellSize];
		
		NSSize amountSize = [m_amount sizeWithAttributes:attrs];
		
		textRect.origin.x = cellFrame.size.width - amountSize.width + cellFrame.origin.x;
		textRect.origin.y += 3;
		textRect.size.width = cellFrame.size.width - (textRect.origin.x) + cellFrame.origin.x;
		
		// only draw it if we have the space
		
		float spaceToDraw = cellFrame.size.width - cellFrame.origin.x - cellSize.width + 20;
		
		if (spaceToDraw >= amountSize.width)
		{
			[m_amount drawInRect:textRect withAttributes:attrs];
		}
	}
	
	[super drawWithFrame:cellFrame inView:controlView];
}

- (void)setAmount:(NSString*)amount
{
	[amount retain];
	[m_amount release];
	m_amount = amount;
}

- (void)dealloc
{
	[super dealloc];
}

@end
