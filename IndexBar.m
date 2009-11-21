//
//  IndexBar.m
//  Stash
//
//  Created by Peter Pearson on 01/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "IndexBar.h"
#import "IndexItem.h"

@implementation IndexBar

- (void)awakeFromNib
{
	m_aSections = [[NSMutableArray alloc] init];
	m_dItems = [[NSMutableDictionary alloc] init];
	
	[self setDataSource:self];
	[self setDelegate:self];	
}

- (void)dealloc
{
	[m_aSections release];
	[m_dItems release];
	
	[super dealloc];
}

- (void)clearAllItems
{
	[m_aSections removeAllObjects];
	[m_dItems removeAllObjects];
}

- (void)addSection:(id)key title:(NSString*)sTitle
{
	IndexItem *section = [[IndexItem alloc] init];
	
	[section setTitle:sTitle];
	[section setItemKey:key];
	[section setSection:YES];
	[section setType:0];
	
	[m_dItems setObject:section forKey:key];
	[m_aSections addObject:section];
	[section release];	
}

- (void)addItem:(id)parentKey key:(id)key title:(NSString*)sTitle item:(int)item action:(SEL)selector target:(id)target type:(int)type
{
	IndexItem *newItem = [[IndexItem alloc] init];
	
	[newItem setTitle:sTitle];
	[newItem setItemKey:key];
	[newItem setParentKey:parentKey];
	[newItem setItemIndex:item];
	[newItem setType:type];
	
	[newItem setAction:selector target:target];
	[m_dItems setObject:newItem forKey:key];
	
	IndexItem *parentItem = [m_dItems objectForKey:parentKey];
	
	if (parentItem != nil)
	{
		[parentItem addChild:newItem];
	}
}

- (void)selectItem:(id)key
{
	IndexItem *itemToSelect = [m_dItems objectForKey:key];
	
	if (itemToSelect == nil)
		return;
	
	if ([itemToSelect numberOfChildren] != 0)
		return;
	
	[self selectRowIndexes:[NSIndexSet indexSetWithIndex:[self rowForItem:itemToSelect]] byExtendingSelection:NO];
}

- (void)expandSection:(id)itemKey
{
	IndexItem *itemToExpand = [m_dItems objectForKey:itemKey];
	
	if (itemToExpand != nil)
	{
		[self expandItem:itemToExpand];	
	}	
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if (item == nil)
	{
		return [m_aSections objectAtIndex:index];
	}
	else
	{
		return [(IndexItem*)item childAtIndex:index];
    }
}

- (BOOL)outlineView:(NSOutlineView*)outlineView isGroupItem:(id)item
{
	return [item isSection];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
	return ![item isSection];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [item isSection];
}

-(NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (item == nil)
	{
		return [m_aSections count];
    }
	
    return [item numberOfChildren];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{	
	return [item title];
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
	IndexItem *selectedItem = [self itemAtRow:[self selectedRow]];
	if (selectedItem == nil)
		return;
	
	SEL action = NULL;
	id target = nil;
	
	if ([selectedItem hasAction])
	{
		action = [selectedItem action];
		target = [selectedItem actionTarget];
		
		if (action != NULL)
			[NSThread detachNewThreadSelector:action toTarget:target withObject:selectedItem];
	}	
}

- (NSMenu*)menuForEvent:(NSEvent*)event
{
    int row = [self rowAtPoint: [self convertPoint: [event locationInWindow] fromView: nil]];
	
    if (row >= 0)
    {
        if ([self isRowSelected:row])
		{
			IndexItem *selectedItem = [self itemAtRow:row];
			
			if (selectedItem == nil)
				return nil;
			
			int type = [selectedItem type];
			
			if (type == 1) // account
			{
				return fAccountMenu;
			}
		}
    }
	
	return nil;
}

- (int)getItemIndex
{
	IndexItem *selectedItem = [self itemAtRow:[self selectedRow]];
	if (selectedItem == nil)
		return -1;
	
	return [selectedItem getItemIndex];
}

@end
