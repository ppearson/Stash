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

#import "SIndexSplitView.h"


@implementation SIndexSplitView

- (void)awakeFromNib
{
	[self setDelegate:self];
	indexSubView = [[self subviews] objectAtIndex:0];
	contentSubView = [[self subviews] objectAtIndex:1];
}

- (CGFloat)dividerThickness
{
	return 1.0;
}

- (void)drawDividerInRect:(NSRect)rect
{
	[[NSColor blackColor] set];
	NSRectFill(rect);
}

- (CGFloat)splitView:(NSSplitView *)sender constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)offset
{
	NSRect mainFrame = [[sender superview] frame];
	return mainFrame.size.width - (mainFrame.size.width - 120.0);
}

- (BOOL)splitView:(id)sender canCollapseSubview:(NSView *)subview
{
    return NO;
}

- (void)splitView:(id)sender resizeSubviewsWithOldSize:(NSSize)oldSize
{
	float newHeight = [sender frame].size.height;
	float newWidth = [sender frame].size.width - [indexSubView frame].size.width - [self dividerThickness];
	
	NSRect newFrame = [indexSubView frame];
	newFrame.size.height = newHeight;
	[indexSubView setFrame:newFrame];
	
	newFrame = [contentSubView frame];
	newFrame.size.width = newWidth;
	newFrame.size.height = newHeight;
	[contentSubView setFrame:newFrame];
	
	[sender adjustSubviews];
}


@end
