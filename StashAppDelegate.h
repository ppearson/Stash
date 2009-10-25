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
	IBOutlet NSTextField *Category;
	IBOutlet NSTextField *Amount;
	IBOutlet id DateCntl;
	IBOutlet id Reconciled;
	
	NSMutableArray *m_aIndexItems;
	NSMutableArray *m_aContentItems;
	
	std::vector<Account> m_aAccounts;
	Account *m_pAccount;	
	bool m_bEditing;
	
	int m_nTransactionOffset;
	
	IndexItem *m_SelectedIndex;
	IndexItem *m_SelectedTransaction;
	
	std::string m_DocumentFile;
	bool m_UnsavedChanges;
}

- (void)buildIndexTree;
- (void)buildContentTree;

- (IBAction)AddAccount:(id)sender;
- (IBAction)AddTransaction:(id)sender;
- (IBAction)Delete:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (IBAction)OpenFile:(id)sender;
- (IBAction)SaveFile:(id)sender;
- (IBAction)SaveFileAs:(id)sender;
- (bool)SaveFileTo:(std::string)path;

- (void)TransactionSelectionDidChange:(NSNotification *)notification;
- (void)quitSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

- (IBAction)updateTransaction:(id)sender;

NSDate *convertToNSDate(Date *date);

@property (assign) IBOutlet NSWindow *window;

@end
