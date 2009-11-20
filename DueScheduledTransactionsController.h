//
//  DueScheduledTransactionsController.h
//  Stash
//
//  Created by Peter Pearson on 15/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class StashAppDelegate;

@interface DueScheduledTransactionsController: NSWindowController
{
	IBOutlet NSTableView *fTransactionsList;
	
	NSMutableArray *m_aTransactions;
	
	StashAppDelegate *fMainController;
}

- (id)initWnd:(StashAppDelegate *)controller withArray:(NSMutableArray*)array;

- (void)addTransaction:(id)sender;
- (void)skipTransaction:(id)sender;

@end
