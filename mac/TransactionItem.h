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

#import <Cocoa/Cocoa.h>
#include "transaction.h"
#include "split_transaction.h"

@interface TransactionItem : NSObject {
	
	NSMutableArray *children;
	NSMutableDictionary *data;
	
	int m_trans;
	int m_split;
}

- (int)transaction;
- (void)setTransaction:(int)transaction;

- (int)splitTransaction;
- (void)setSplitTransaction:(int)stransaction;

- (NSString *)keyValue:(NSString *)key;
- (void)setValue:(NSString *)value forKey:(NSString *)key;

- (int)intKeyValue:(NSString *)key;
- (void)setIntValue:(int)value forKey:(NSString *)key;

- (BOOL)boolKeyValue:(NSString *)key;
- (void)setBoolValue:(BOOL)value forKey:(NSString *)key;

- (void)addChild:(TransactionItem *)n;
- (int)childrenCount;
- (TransactionItem *)childAtIndex:(int)i;
- (void)deleteChild:(int)i;

- (void)setChildrenTransactionIndex:(int)index;

- (BOOL)expandable;

@end
