//
//  TransactionsController.h
//  Stash
//
//  Created by Peter Pearson on 29/08/2010.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "account.h"
#include "document.h"
#include "transaction.h"

#import "TransactionItem.h"
#import "MakeTransferController.h"
#import "IndexBar.h"

enum TransactionsToShow
{
	RECENT,
	ALL_THIS_YEAR,
	ALL
};

@interface TransactionsController : NSViewController
{
	NSWindow *window;
	IBOutlet IndexBar *indexBar;
	
	// Transactions
	NSMenu *transactionsTableHeaderMenu;
	IBOutlet NSOutlineView *transactionsTableView;
	IBOutlet NSSplitView *transactionsverticalSplitView;
	IBOutlet NSComboBox *transactionsPayee;
	IBOutlet NSComboBox *transactionsCategory;
	IBOutlet NSTextField *transactionsDescription;
	IBOutlet NSTextField *transactionsAmount;
	IBOutlet NSPopUpButton *transactionsType;
	IBOutlet id transactionsDateCntl;
	IBOutlet id transactionsCleared;
	
	IBOutlet NSButton *addTransaction;
	IBOutlet NSButton *deleteTransaction;
	IBOutlet NSButton *splitTransaction;
	IBOutlet NSButton *moveUp;
	IBOutlet NSButton *moveDown;
	IBOutlet NSButton *refresh;
	
	int m_showTransactionsViewType;
	
	NSMutableArray *m_aTransactionItems;
	
	bool m_bEditing;
	
	int m_nTransactionOffset;
	
	TransactionItem *m_SelectedIndex;
	TransactionItem *m_SelectedTransaction;
	
	
	///
	
	Document *m_pDocument;
	Account *m_pAccount;
	
	std::vector<fixed> m_aBalance;
}

+ (TransactionsController*)sharedInterface;

-(id)retain;
- (NSUInteger)retainCount;
-(void)release;
-(id)autorelease;

-(id)init;

- (void)showTransactionsForAccount:(Account*)account;

- (void)buildTransactionsTree;

- (IBAction)AddTransaction:(id)sender;
- (IBAction)DeleteTransaction:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (NSString*)transactionTypeToString:(TransactionType)type;
- (NSString*)convertUSNegAmount:(NSString*)string;

- (IBAction)MoveUp:(id)sender;
- (IBAction)MoveDown:(id)sender;
- (void)SwapTransactions:(int)from to:(int)to;
- (IBAction)RefreshView:(id)sender;

- (void)TransactionSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateTransaction:(id)sender;

- (TransactionItem*)createTransactionItem:(Transaction&)transaction index:(int)index;

- (void)transactionTableColumnMenu:(id)sender;

NSDate *convertToNSDate(Date &date);
NSDate *convertToNSDate(MonthYear &date);

- (void)updateBalancesFromTransactionIndex:(int)nIndex;
- (void)updateTransactionsFromTransactionIndex:(int)nIndex;

- (void)handleTransactionsSettingsUpdate:(NSNotification *)note;

- (void)setTransactionsViewType:(int)type;

- (void)cleanUp;

- (void)setWindow:(NSWindow*)wnd;
- (void)setIndexBar:(IndexBar*)bar;

- (void)refreshLibraryItems;
- (void)makeTransferItem:(MakeTransferController *)makeTransferCont;


@end
