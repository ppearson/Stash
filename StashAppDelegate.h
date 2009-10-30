//
//  StashAppDelegate.h
//  Stash
//
//  Created by Peter Pearson on 24/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "account.h"
#include "document.h"
#include "transaction.h"

#import "AddAccountController.h"
#import "IndexItem.h"

@interface StashAppDelegate : NSObject// <NSApplicationDelegate>
{
    NSWindow *window;
	
	IBOutlet NSView *indexViewPlaceholder;
	IBOutlet NSView *contentViewPlaceholder;
	
	// Index View
	IBOutlet NSView *indexView;
	IBOutlet NSOutlineView *indexTableView;
	
	// Content View
	IBOutlet NSView *contentView;
	IBOutlet NSOutlineView *transactionsTableView;
		
	IBOutlet NSSplitView  *verticalSplitView;
			
	IBOutlet NSTextField *Payee;
	IBOutlet NSTextField *Description;
	IBOutlet NSTextField *Category;
	IBOutlet NSTextField *Amount;
	
	IBOutlet NSPopUpButton *Type;
	IBOutlet id DateCntl;
	IBOutlet id Reconciled;
	
	NSMutableArray *m_aIndexItems;
	NSMutableArray *m_aContentItems;
	
	Document m_Document;
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

- (void)updateUI;

- (IBAction)AddAccount:(id)sender;
- (IBAction)AddTransaction:(id)sender;
- (IBAction)Delete:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (void)addAccountConfirmed:(AddAccountController *)addAccountController;

- (IBAction)OpenFile:(id)sender;
- (IBAction)SaveFile:(id)sender;
- (IBAction)SaveFileAs:(id)sender;
- (bool)OpenFileAt:(std::string)path;
- (bool)SaveFileTo:(std::string)path;

- (IBAction)ImportQIF:(id)sender;
- (IBAction)ExportQIF:(id)sender;

- (void)IndexSelectionDidChange:(NSNotification *)notification;
- (void)TransactionSelectionDidChange:(NSNotification *)notification;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

- (IBAction)updateTransaction:(id)sender;

NSDate *convertToNSDate(Date *date);

@property (assign) IBOutlet NSWindow *window;

@end
