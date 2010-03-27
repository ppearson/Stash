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

#import "DueScheduledTransactionsController.h"


@implementation DueScheduledTransactionsController

- (id)initWnd:(StashAppDelegate *)controller withArray:(NSMutableArray*)array
{
	if ((self = [super initWithWindowNibName:@"DueScheduledTransactions"]))
	{
		fMainController = controller;
		
		m_aTransactions = [[NSMutableArray alloc] initWithArray:array];
	}
	
	return self;	
}

- (void)awakeFromNib
{
	[fTransactionsList reloadData];
	
	[fTransactionsList selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
}

- (void)dealloc
{
	[m_aTransactions release];
	
	[super dealloc];
}

- (id)tableView:(NSTableView *) aTableView objectValueForTableColumn:(NSTableColumn *) aTableColumn row:(NSInteger) rowIndex
{
	id result = @"";
	NSString *identifier = [aTableColumn identifier];
	
	NSMutableDictionary *oObject = [m_aTransactions objectAtIndex:rowIndex];
		
	result = [oObject valueForKey:identifier];
		
	return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [m_aTransactions count];
	
	return 0;
}

- (void)addTransaction:(id)sender
{
	// get the SchedTrans index
	NSInteger row = [fTransactionsList selectedRow];
	NSMutableDictionary *oObject = [m_aTransactions objectAtIndex:row];
	
	int nSchedTransIndex = [[oObject valueForKey:@"index"] intValue];
	
	if (nSchedTransIndex < 0)
		return;
	
	[fMainController AddDueScheduledTransaction:nSchedTransIndex];
	
	[m_aTransactions removeObjectAtIndex:row];
	
	[fTransactionsList reloadData];
	
	if ([m_aTransactions count] == 0)
	{
		[[self window] performClose:sender];
	}
}

- (void)skipTransaction:(id)sender
{
	// get the SchedTrans index
	NSInteger row = [fTransactionsList selectedRow];
	NSMutableDictionary *oObject = [m_aTransactions objectAtIndex:row];
	
	int nSchedTransIndex = [[oObject valueForKey:@"index"] intValue];
	
	if (nSchedTransIndex < 0)
		return;
	
	[fMainController SkipDueScheduledTransaction:nSchedTransIndex];
	
	[m_aTransactions removeObjectAtIndex:row];
	
	[fTransactionsList reloadData];
	
	if ([m_aTransactions count] == 0)
	{
		[[self window] performClose:sender];
	}
}

- (void)closeWindow:(id)sender
{
	[[self window] performClose:sender];
}

@end
