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

#import "IndexItem.h"

@implementation IndexItem

@synthesize parentKey;
@synthesize itemKey;
@synthesize type;
@synthesize title;

@synthesize actionTarget;
@synthesize action;

@synthesize renameTarget;
@synthesize rename;

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

- (void)setRename:(SEL)selector target:(id)target
{
	renameTarget = target;
	rename = selector;
}

- (BOOL)hasRename
{
	return rename != NULL;
}


@end
