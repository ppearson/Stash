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

#import "TransactionItem.h"

@implementation TransactionItem

- (id)init
{
	[super init];
	children = [[NSMutableArray alloc] init];
	data = [[NSMutableDictionary alloc] init];
	
	m_trans = -1;
	m_split = -1;
	
	return self;
}

- (int)transaction
{
	return m_trans;
}

- (void)setTransaction:(int)transaction
{
	m_trans = transaction;
}

- (int)splitTransaction
{
	return m_split;
}

- (void)setSplitTransaction:(int)stransaction
{
	m_split = stransaction;
}

- (void)setValue:(NSString *)value forKey:(NSString *)key
{
	[data setValue:value forKey:key];
}

- (NSString *)keyValue:(NSString *)key
{
	if ([data objectForKey:key] != nil)
		return [data valueForKey:key];
	
	return @"";
}

- (int)intKeyValue:(NSString *)key
{
	if ([data objectForKey:key] != nil)
		return [[data valueForKey:key] intValue];
	
	return -1;
}

- (void)setIntValue:(int)value forKey:(NSString *)key
{
	[data setValue:[NSNumber numberWithInt:value] forKey:key];
}

- (BOOL)boolKeyValue:(NSString *)key
{
	if ([data objectForKey:key] != nil)
		return [[data valueForKey:key] boolValue];
	
	return NO;
}

- (void)setBoolValue:(BOOL)value forKey:(NSString *)key
{
	[data setValue:[NSNumber numberWithBool:value] forKey:key];
}

- (void)addChild:(TransactionItem *)n
{
	[children addObject:n];
}

- (TransactionItem *)childAtIndex:(int)i
{
	return [children objectAtIndex:i];
}

- (int)childrenCount
{
	return [children count];
}

- (void)deleteChild:(int)i
{
	[children removeObjectAtIndex:i];
}

- (BOOL)expandable
{
	return ([children count] > 0);
}

- (void)dealloc
{
	[children release];
	[data release];
	[super dealloc];
}

@end
