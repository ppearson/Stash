//
//  STransactionsSplitView.m
//  Stash
//
//  Created by Peter Pearson on 30/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "STransactionsSplitView.h"

@implementation STransactionsSplitView

- (void)awakeFromNib
{
	[super awakeFromNib];
	[self setDelegate:self];
	
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
	return mainFrame.size.height - (mainFrame.size.height - 100.0);
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMax ofSubviewAt:(NSInteger)index
{
	NSRect mainFrame = [[splitView superview] frame];
	return mainFrame.size.height - 200;
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
