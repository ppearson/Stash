//
//  StashAppDelegate.h
//  Stash
//
//  Created by Peter Pearson on 24/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "account.h"
#include "transaction.h"
#import "IndexItem.h"

@interface StashAppDelegate : NSObject <NSApplicationDelegate>
{
    NSWindow *window;
	IBOutlet NSSplitView *splitView;
	IBOutlet NSOutlineView *indexView, *contentView;
	
	IBOutlet NSTextField *Payee;
	IBOutlet NSTextField *Description;
	IBOutlet NSTextField *Amount;
	
	IBOutlet id DateCntl;
	
	NSMutableArray *m_aIndexItems;
	NSMutableArray *m_aContentItems;
	
	std::vector<Account> m_aAccounts;
	Account *m_pAccount;	
	bool m_bEditing;
	
	IndexItem *m_SelectedIndex;
	IndexItem *m_SelectedTransaction;
}

- (void)buildIndexTree;
- (void)buildContentTree;

- (IBAction)AddAccount:(id)sender;
- (IBAction)AddTransaction:(id)sender;
- (IBAction)Delete:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (void)TransactionSelectionDidChange:(NSNotification *)notification;

- (IBAction)updateItem:(id)sender;

NSDate *convertToNSDate(Date *date);

@property (assign) IBOutlet NSWindow *window;

@end
