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
#include "account.h"
#include "document.h"
#include "transaction.h"

#import "AddAccountController.h"
#import "ImportQIFController.h"
#import "TransactionItem.h"
#import "IndexBar.h"
#import "DueScheduledTransactionsController.h"
#import "PreferencesController.h"
#import "PieChartView.h"
#import "AreaChartView.h"
#import "MakeTransfer.h"
#import "ImportOFXController.h"
#import "ExportOFXController.h"

enum TransactionsToShow
{
	RECENT,
	ALL_THIS_YEAR,
	ALL
};

@interface StashAppDelegate : NSObject
{
    NSWindow *window;
	
	IBOutlet NSView *indexViewPlaceholder;
	IBOutlet NSView *contentViewPlaceholder;
	
	// Index View
	IBOutlet NSView *indexView;
	IBOutlet IndexBar *indexBar;
	
	IBOutlet NSSplitView *indexBarSplitView;
	
	// Content Views
	IBOutlet NSView *contentView;	
	IBOutlet NSView *vTransactionsView, *vPayeesView, *vCategoriesView, *vScheduledView, *vGraphView;
	
	int nViewType;
	
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
	IBOutlet id transactionsReconciled;
	
	IBOutlet NSButton *addTransaction;
	IBOutlet NSButton *deleteTransaction;
	IBOutlet NSButton *splitTransaction;
	IBOutlet NSButton *moveUp;
	IBOutlet NSButton *moveDown;
	IBOutlet NSButton *refresh;
	
	int ShowTransactionsViewType;
	
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
	IBOutlet NSPopUpButton *scheduledConstraint;
	IBOutlet id scheduledDateCntl;
	
	IBOutlet NSButton *deleteScheduled;
	
	NSMutableArray *m_aScheduledTransactions;
		
	// Graphs
	
	IBOutlet NSSegmentedControl* viewingPeriodSegmentControl;
	IBOutlet PieChartView *pieChartView;
	IBOutlet AreaChartView *areaChartView;
	IBOutlet NSPopUpButton *graphAccount;
	IBOutlet NSPopUpButton *graphType;
	IBOutlet id graphStartDateCntrl;
	IBOutlet id graphEndDateCntrl;
	IBOutlet id graphIgnoreTransfers;
	
	PreferencesController *prefController;
	MakeTransfer *makeTransfer;
	ImportOFXController *importOFXController;
	ExportOFXController *exportOFXController;	
	
	Document m_Document;
	Account *m_pAccount;
	
	std::vector<fixed> m_aBalance;
	
	Graph *m_pGraph;
	int m_nGraphDateSegment;
	
	std::string m_DocumentFile;
	bool m_UnsavedChanges;
	
	NSString *m_sPendingOpenFile;
	bool m_HasFinishedLoading;
}

@property (assign) int ShowTransactionsViewType;

- (void)buildIndexTree;
- (void)buildTransactionsTree;
- (void)buildPayeesList;
- (void)buildCategoriesList;
- (void)buildSchedTransList;
- (void)buildGraph:(int)account startDate:(NSDate*)startDate endDate:(NSDate*)endDate type:(GraphType)type ignoreTransfers:(bool)ignoreTransfers;
- (void)refreshLibraryItems;

- (void)setWindowTitleWithDocName:(NSString*)path;

- (void)calculateAndShowScheduled;
- (void)AddDueScheduledTransaction:(int)index;
- (void)SkipDueScheduledTransaction:(int)index;

// Transactions

- (IBAction)AddAccount:(id)sender;
- (IBAction)AccountInfo:(id)sender;

- (void)updateAccountInfo:(int)account name:(NSString*)name institution:(NSString*)institution
				   number:(NSString*)number note:(NSString*)note type:(AccountType)type;

- (IBAction)DeleteAccount:(id)sender;
- (IBAction)AddTransaction:(id)sender;
- (IBAction)DeleteTransaction:(id)sender;
- (IBAction)SplitTransaction:(id)sender;

- (IBAction)MakeTransfer:(id)sender;
- (void)makeTransferItem:(MakeTransfer *)makeTransferController;

- (NSString*)transactionTypeToString:(TransactionType)type;
- (NSString*)convertUSNegAmount:(NSString*)string;

- (IBAction)MoveUp:(id)sender;
- (IBAction)MoveDown:(id)sender;
- (void)SwapTransactions:(int)from to:(int)to;
- (void)RefreshView:(id)sender;

- (TransactionItem*)createTransactionItem:(Transaction&)transaction index:(int)index;

- (IBAction)showRecentTransactions:(id)sender;
- (IBAction)showAllTransactionsThisYear:(id)sender;
- (IBAction)showAllTransactions:(id)sender;

- (void)viewToolbarClicked:(id)sender;
- (void)transactionTableColumnMenu:(id)sender;


// Payees

- (IBAction)DeletePayee:(id)sender;

- (IBAction)DeleteCategory:(id)sender;

- (IBAction)AddScheduledTransaction:(id)sender;
- (IBAction)DeleteScheduledTransaction:(id)sender;

- (void)accountSelected:(id)sender;
- (void)payeesSelected:(id)sender;
- (void)categoriesSelected:(id)sender;
- (void)scheduledSelected:(id)sender;
- (void)graphSelected:(id)sender;

- (void)accountRenamed:(id)sender;
- (void)graphRenamed:(id)sender;

- (IBAction)deleteMisc:(id)sender;

- (void)addAccountConfirmed:(AddAccountController *)addAccountController;

- (IBAction)AddGraph:(id)sender;

- (IBAction)NewFile:(id)sender;
- (IBAction)OpenFile:(id)sender;
- (IBAction)SaveFile:(id)sender;
- (IBAction)SaveFileAs:(id)sender;
- (bool)OpenFileAt:(std::string)path;
- (bool)SaveFileTo:(std::string)path;

- (IBAction)ImportQIF:(id)sender;
- (void)importQIFConfirmed:(ImportQIFController *)importQIFController;

- (IBAction)ImportOFX:(id)sender;
- (void)importOFXFileWithController:(ImportOFXController*)controller reverseTransactions:(bool)reverse reconciled:(bool)reconciled ignoreExisting:(bool)ignoreExisting;
- (void)deleteImportOFXController;

- (IBAction)ExportOFX:(id)sender;
- (void)exportOFXFileWithController:(ExportOFXController*)controller xmlOFX:(bool)xml;
- (void)deleteExportOFXController;

- (IBAction)ExportQIF:(id)sender;

- (void)TransactionSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateTransaction:(id)sender;

- (void)ScheduledSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateScheduled:(id)sender;

- (IBAction)redrawGraph:(id)sender;
- (IBAction)dateBarClicked:(id)sender;
- (IBAction)graphDatesManuallyChanged:(id)sender;

- (IBAction)updateGraph:(id)sender;

- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem;

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

- (IBAction)showPreferencesWindow:(id)sender;

NSDate *convertToNSDate(Date &date);
NSDate *convertToNSDate(MonthYear &date);

- (void)updateBalancesFromTransactionIndex:(int)nIndex;
- (void)updateTransactionsFromTransactionIndex:(int)nIndex;

- (void)handleTransactionsSettingsUpdate:(NSNotification *)note;
- (void)handleGraphSettingsUpdate:(NSNotification *)note;

@property (assign) IBOutlet NSWindow *window;

@end
