//
//  DueScheduledTransactionsController.m
//  Stash
//
//  Created by Peter Pearson on 15/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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
}

- (void)dealloc
{
	
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

@end
