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

#import "STransactionsSplitView.h"

@implementation STransactionsSplitView

- (void)awakeFromNib
{
	[self setDelegate:(id)self];
	
	transactionsSubView = [[self subviews] objectAtIndex:0];
	editSubView = [[self subviews] objectAtIndex:1];
}

- (CGFloat)dividerThickness
{
	return 1.0;
}

- (void)drawDividerInRect:(NSRect)rect
{
	[[NSColor grayColor] set];
	NSRectFill(rect);
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)index
{
	NSRect mainFrame = [[splitView superview] frame];
	return mainFrame.size.height - (mainFrame.size.height - 120.0);
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMax ofSubviewAt:(NSInteger)index
{
	NSRect mainFrame = [[splitView superview] frame];
	return mainFrame.size.height - 175;
}

- (BOOL)splitView:(id)sender canCollapseSubview:(NSView *)subview
{
	return (subview == editSubView);
}

- (void)splitView:(id)sender resizeSubviewsWithOldSize:(NSSize)oldSize
{
	float newHeight = [sender frame].size.height - [editSubView frame].size.height - [self dividerThickness];
	float newWidth = [sender frame].size.width;
	
	NSRect newFrame = [editSubView frame];
	newFrame.size.width = newWidth;
	[editSubView setFrame:newFrame];
	
	newFrame = [transactionsSubView frame];
	newFrame.size.width = newWidth;
	newFrame.size.height = newHeight;
	
	[transactionsSubView setFrame:newFrame];
	
	[sender adjustSubviews];
}

@end
