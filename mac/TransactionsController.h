/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2010 Peter Pearson
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
#include "account.h"
#include "document.h"
#include "transaction.h"

#import "TransactionItem.h"
#import "MakeTransferController.h"
#import "IndexBar.h"

@class StashAppDelegate;

enum TransactionsToShow
{
	RECENT,
	ALL_THIS_YEAR,
	ALL
};

@interface TransactionsController : NSViewController <NSOutlineViewDataSource, NSOutlineViewDelegate>
{
	StashAppDelegate *fMainController;
	
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
	
	unsigned int m_nTransactionOffset;
	
	TransactionItem *m_SelectedIndex;
	TransactionItem *m_SelectedTransaction;	
	
	///
	
	Document *m_pDocument;
	Account *m_pAccount;
	
	std::vector<fixed> m_aBalance;

	NSFont*		m_monoSpaceFont;
}

+ (TransactionsController*)sharedInterface;

-(id)retain;
- (NSUInteger)retainCount;
-(void)release;
-(id)autorelease;

-(id)init;

- (void)setMainController:(StashAppDelegate *)controller;

- (void)showTransactionsForAccount:(Account*)account;

- (void)buildTransactionsTree;

- (IBAction)AddTransaction:(id)sender;
- (IBAction)DeleteTransaction:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (NSString*)transactionTypeToString:(Transaction::Type)type;

- (IBAction)MoveUp:(id)sender;
- (IBAction)MoveDown:(id)sender;
- (void)SwapTransactions:(unsigned int)from to:(unsigned int)to;
- (IBAction)RefreshView:(id)sender;

- (void)TransactionSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateTransaction:(id)sender;

- (TransactionItem*)createTransactionItem:(Transaction&)transaction index:(int)index;

- (void)transactionTableColumnMenu:(id)sender;

// stand alone, but inefficient version...
NSDate* convertToNSDate(const Date& date);

// more efficient version
NSDate* convertToNSDate(const Date& date, NSCalendar* gregorian, NSDateComponents* dateComponents);

- (void)updateBalancesFromTransactionIndex:(unsigned int)nIndex;
- (void)updateTransactionsFromTransactionIndex:(unsigned int)nIndex;

- (void)handleTransactionsSettingsUpdate:(NSNotification *)note;

- (void)setTransactionsViewType:(int)type;

- (void)cleanUp;

- (void)setWindow:(NSWindow*)wnd;
- (void)setIndexBar:(IndexBar*)bar;

- (void)refreshLibraryItems;
- (void)makeTransferItem:(MakeTransferController *)makeTransferCont;

- (void)setDocumentModified:(BOOL)modified;

@end
