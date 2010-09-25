/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009-2010 Peter Pearson
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

#import "IndexBar.h"
#import "IndexItem.h"
#import "IndexBarTextCell.h"

@implementation IndexBar

- (void)awakeFromNib
{
	m_aSections = [[NSMutableArray alloc] init];
	m_dItems = [[NSMutableDictionary alloc] init];
	
	[self setDataSource:self];
	[self setDelegate:self];
	
	NSTableColumn *tableColumn;
	IndexBarTextCell *customCell;
	
	// Our folders have images next to them.
	tableColumn = [self tableColumnWithIdentifier:@"Name"];
	customCell = [[[IndexBarTextCell alloc] init] autorelease];
	[customCell setEditable:YES];
	
	// we have to copy the font across from the old cell, otherwise it doesn't take notice of
	// the xib's control size of small
	NSCell *pOldCell = [tableColumn dataCell];
	[customCell setFont:[pOldCell font]];
	
	[tableColumn setDataCell:customCell];
}

- (void)dealloc
{
	[self clearAllItems];
	
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
	[section setAmount:@""];
	[section setItemKey:key];
	[section setSection:YES];
	[section setType:0];
	
	[m_dItems setObject:section forKey:key];
	[m_aSections addObject:section];
	[section release];	
}

- (void)addItem:(id)parentKey key:(id)key title:(NSString*)sTitle item:(int)item action:(SEL)selector target:(id)target type:(int)type rename:(SEL)renamer renameTarget:(id)reTarget
{
	IndexItem *newItem = [[IndexItem alloc] init];
	
	[newItem setTitle:sTitle];
	[newItem setAmount:@""];
	[newItem setItemKey:key];
	[newItem setParentKey:parentKey];
	[newItem setItemIndex:item];
	[newItem setType:type];
	
	[newItem setAction:selector target:target];
	[m_dItems setObject:newItem forKey:key];
	
	[newItem setRename:renamer target:reTarget];
	
	IndexItem *parentItem = [m_dItems objectForKey:parentKey];
	
	if (parentItem != nil)
	{
		[parentItem addChild:newItem];
	}
	
	[newItem release];
}

- (void)addItem:(id)parentKey key:(id)key title:(NSString*)sTitle amount:(NSString*)sAmount item:(int)item action:(SEL)selector target:(id)target type:(int)type rename:(SEL)renamer renameTarget:(id)reTarget
{
	IndexItem *newItem = [[IndexItem alloc] init];
	
	[newItem setTitle:sTitle];
	[newItem setAmount:sAmount];
	
	[newItem setItemKey:key];
	[newItem setParentKey:parentKey];
	[newItem setItemIndex:item];
	[newItem setType:type];
	
	[newItem setAction:selector target:target];
	[m_dItems setObject:newItem forKey:key];
	
	[newItem setRename:renamer target:reTarget];
	
	IndexItem *parentItem = [m_dItems objectForKey:parentKey];
	
	if (parentItem != nil)
	{
		[parentItem addChild:newItem];
	}
	
	[newItem release];
}

- (void)updateAmount:(id)key amount:(NSString*)sAmount
{
	IndexItem *item = [m_dItems objectForKey:key];
	
	if (item == nil)
		return;
	
	[item setAmount:sAmount];
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
	if ([[tableColumn identifier] isEqualToString:@"Name"])
	{
		return [item title];
	}
	
	return [item amount];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	BOOL result = NO;
	
	if ([[tableColumn identifier] isEqualToString:@"Name"])
	{
		result = [item hasRename];
	}
	
	return result;
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{	
	if (item)
	{
		SEL action = NULL;
		id target = nil;
		
		if ([item hasRename])
		{
			action = [item rename];
			target = [item renameTarget];
			
			[item setTitle:object];
			
			if (action != NULL)
			{
				[target performSelector:action withObject:item];
			}
		}
	}
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
			[target performSelector:action withObject:selectedItem];
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
			else if (type == 3) // graph
			{
				return fMiscMenu;
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

- (id)getSelectedItemKey
{
	IndexItem *selectedItem = [self itemAtRow:[self selectedRow]];
	if (selectedItem == nil)
		return nil;
	
	return [selectedItem itemKey];	
}

- (IBAction)renameItem:(id)key
{
	int row = -1;
	
	if ([key isKindOfClass: [NSMenuItem class]])
	{
		row = [self selectedRow];
	}
	else
	{
		IndexItem *itemToRename = [m_dItems objectForKey:key];
		
		if (itemToRename == nil)
			return;
		
		row = [self rowForItem:itemToRename];
	}
	
	if (row >= 0)
	{
		[self editColumn:0 row:row withEvent:nil select:YES];
	}
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item 
{
	if ([[tableColumn identifier] isEqualToString:@"Name"]) 
	{
		IndexItem *thisItem = (IndexItem *)item;

		IndexBarTextCell *realCell = (IndexBarTextCell *)cell;
		
		NSString *sAmount = [thisItem amount];
		
		[realCell setAmount:sAmount];
	}
}

@end
