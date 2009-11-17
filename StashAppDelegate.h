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
#import "ImportQIFController.h"
#import "TransactionItem.h"
#import "IndexBar.h"
#import "DueScheduledTransactionsController.h"

enum TransactionsToShow
{
	LAST_100,
	ALL_THIS_YEAR,
	ALL
};

@interface StashAppDelegate : NSObject// <NSApplicationDelegate>
{
    NSWindow *window;
	
	IBOutlet NSView *indexViewPlaceholder;
	IBOutlet NSView *contentViewPlaceholder;
	
	// Index View
	IBOutlet NSView *indexView;
	IBOutlet IndexBar *indexBar;
	
	// Content Views
	IBOutlet NSView *contentView;
	
	IBOutlet NSView *vTransactionsView, *vPayeesView, *vCategoriesView, *vScheduledView;
	
	// Transactions	
	IBOutlet NSOutlineView *transactionsTableView;
	IBOutlet NSSplitView  *transactionsverticalSplitView;
	IBOutlet NSComboBox *transactionsPayee;
	IBOutlet NSComboBox *transactionsCategory;
	IBOutlet NSTextField *transactionsDescription;
	IBOutlet NSTextField *transactionsAmount;
	IBOutlet NSPopUpButton *transactionsType;
	IBOutlet id transactionsDateCntl;
	IBOutlet id transactionsReconciled;
	
	IBOutlet NSButton *addTransaction;
	IBOutlet NSButton *deleteTransaction;
	IBOutlet NSButton *splitTransaction;
	IBOutlet NSButton *moveUp;
	IBOutlet NSButton *moveDown;
	IBOutlet NSButton *refresh;
	
	TransactionsToShow nShowTransactionsType;
	
	NSMutableArray *m_aTransactionItems;
	
	bool m_bEditing;
	
	int m_nTransactionOffset;
	
	TransactionItem *m_SelectedIndex;
	TransactionItem *m_SelectedTransaction;
	
	
	// Payees
	IBOutlet NSTableView *payeesTableView;
	NSMutableArray *m_aPayeeItems;
	
	
	// Categories
	IBOutlet NSTableView *categoriesTableView;
	NSMutableArray *m_aCategoryItems;
	
	
	// Scheduled Transactions
	IBOutlet NSTableView *scheduledTransactionsTableView;
	IBOutlet NSSplitView  *scheduledverticalSplitView;
	IBOutlet NSComboBox *scheduledPayee;
	IBOutlet NSComboBox *scheduledCategory;
	IBOutlet NSTextField *scheduledDescription;
	IBOutlet NSTextField *scheduledAmount;
	IBOutlet NSPopUpButton *scheduledAccount;
	IBOutlet NSPopUpButton *scheduledType;
	IBOutlet NSPopUpButton *scheduledFrequency;
	IBOutlet id scheduledDateCntl;
	
	IBOutlet NSButton *deleteScheduled;
	
	NSMutableArray *m_aScheduledTransactions;
	
	
	Document m_Document;
	Account *m_pAccount;	
	
	
	std::string m_DocumentFile;
	bool m_UnsavedChanges;
	
	NSString *m_sPendingOpenFile;
	bool m_HasFinishedLoading;
}

- (void)buildIndexTree;
- (void)buildTransactionsTree;
- (void)buildPayeesList;
- (void)buildCategoriesList;
- (void)buildSchedTransList;
- (void)refreshLibraryItems;

- (void)updateUI;

- (void)calculateAndShowScheduled;
- (void)AddDueScheduledTransaction:(int)index;
- (void)SkipDueScheduledTransaction:(int)index;

// Transactions

- (IBAction)AddAccount:(id)sender;
- (IBAction)AddTransaction:(id)sender;
- (IBAction)DeleteTransaction:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (IBAction)MoveUp:(id)sender;
- (IBAction)MoveDown:(id)sender;
- (void)SwapTransactions:(int)from to:(int)to;
- (void)RefreshView:(id)sender;

- (IBAction)showLast100Transactions:(id)sender;
- (IBAction)showAllTransactionsThisYear:(id)sender;
- (IBAction)showAllTransactions:(id)sender;


// Payees

- (IBAction)DeletePayee:(id)sender;

- (IBAction)DeleteCategory:(id)sender;

- (IBAction)AddScheduledTransaction:(id)sender;
- (IBAction)DeleteScheduledTransaction:(id)sender;

- (void)accountSelected:(id)sender;
- (void)payeesSelected:(id)sender;
- (void)categoriesSelected:(id)sender;
- (void)scheduledSelected:(id)sender;

- (void)addAccountConfirmed:(AddAccountController *)addAccountController;

- (IBAction)OpenFile:(id)sender;
- (IBAction)SaveFile:(id)sender;
- (IBAction)SaveFileAs:(id)sender;
- (bool)OpenFileAt:(std::string)path;
- (bool)SaveFileTo:(std::string)path;

- (IBAction)ImportQIF:(id)sender;
- (IBAction)ExportQIF:(id)sender;

- (void)importQIFConfirmed:(ImportQIFController *)importQIFController;

- (void)TransactionSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateTransaction:(id)sender;

- (void)ScheduledSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateScheduled:(id)sender;

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

NSDate *convertToNSDate(Date &date);

@property (assign) IBOutlet NSWindow *window;

@end
