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

- (float)splitView:(NSSplitView *)splitView constrainMinCoordinate:(float)proposedMin ofSubviewAt:(int)index
{
	return (proposedMin + 150.0);	
}

/*
- (float)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(float)proposedMax ofSubviewAt:(int)index
{
	return (proposedMax - 350.0);	
}

*/

- (void)splitView:(id)sender resizeSubviewsWithOldSize:(NSSize)oldSize
{
	float newWidth = [sender frame].size.width - [indexSubView frame].size.width - [self dividerThickness];
	
	NSRect newFrame = [indexSubView frame];
	[indexSubView setFrame:newFrame];
	
	newFrame = [contentSubView frame];
	newFrame.size.width = newWidth;
	[contentSubView setFrame:newFrame];
	
	[sender adjustSubviews];
}


@end
