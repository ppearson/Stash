//
//  SIndexSplitView.m
//  Stash
//
//  Created by Peter Pearson on 30/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "SIndexSplitView.h"


@implementation SIndexSplitView

- (void)awakeFromNib
{
	[super awakeFromNib];
	[self setDelegate:self];
	indexSubView = [[self subviews] objectAtIndex:0];
	contentSubView = [[self subviews] objectAtIndex:1];
}

- (float)dividerThickness
{
	return 1.0;
}

- (void)drawDividerInRect:(NSRect)rect
{
	[[NSColor blackColor] set];
	NSRectFill(rect);
}

- (float)splitView:(NSSplitView *)sender constrainMinCoordinate:(float)proposedMin ofSubviewAt:(int)offset
{
	NSRect mainFrame = [[sender superview] frame];
	return mainFrame.size.width - (mainFrame.size.width - 150.0);
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
