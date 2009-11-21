//
//  IndexItem.m
//  Stash
//
//  Created by Peter Pearson on 01/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "IndexItem.h"

@implementation IndexItem

@synthesize parentKey;
@synthesize itemKey;
@synthesize type;
@synthesize title;

@synthesize actionTarget;
@synthesize action;

- (id)init
{
	if ((self = [super init]))
	{
		m_aChildren = [[NSMutableArray alloc] init];
	}
	
	return self;
}

- (void)dealloc
{
	[m_aChildren release];
	[title release];
		
	[super dealloc];
}

- (void)addChild:(IndexItem *)item
{
	[m_aChildren addObject:item];	
}

- (IndexItem *)childAtIndex:(int)index
{
	return [m_aChildren objectAtIndex:index];
}

- (int)numberOfChildren
{
	return [m_aChildren count];
}

- (void)setItemIndex:(int)iIndex
{
	itemIndex = iIndex;
}

- (int)getItemIndex
{
	return itemIndex;
}

- (void)setSection:(BOOL)bSection
{
	m_section = bSection;
}

- (BOOL)isSection
{
	return m_section;
}

- (void)setAction:(SEL)selector target:(id)target
{
	actionTarget = target;
	action = selector;
}

- (BOOL)hasAction
{
	return action != NULL;
}


@end
