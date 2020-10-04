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
#import "MakeTransferController.h"
#import "ImportOFXController.h"
#import "ExportOFXController.h"

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
	bool m_bEditing;
	
	int ShowTransactionsViewType;
		
	
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
	
	PreferencesController *prefController;
	MakeTransferController *makeTransferController;
	ImportOFXController *importOFXController;
	ExportOFXController *exportOFXController;	
	
	Document m_Document;
	Account *m_pAccount;
	
	std::vector<fixed> m_aBalance;
	
	std::string m_DocumentFile;
	
	NSString *m_sPendingOpenFile;
	bool m_HasFinishedLoading;
    
    
}

@property (assign) int ShowTransactionsViewType;

- (void)buildIndexTree;

- (void)buildPayeesList;
- (void)buildCategoriesList;
- (void)buildSchedTransList;
- (void)refreshLibraryItems;

- (void)setWindowTitleWithDocName:(NSString*)path;

- (void)calculateAndShowScheduled;
- (void)AddDueScheduledTransaction:(int)index;
- (void)SkipDueScheduledTransaction:(int)index;

// Transactions

- (IBAction)AddAccount:(id)sender;
- (IBAction)AccountInfo:(id)sender;

- (void)updateAccountInfo:(int)account name:(NSString*)name institution:(NSString*)institution
				   number:(NSString*)number note:(NSString*)note type:(Account::Type)type;

- (IBAction)DeleteAccount:(id)sender;

- (IBAction)MakeTransfer:(id)sender;
- (void)makeTransferItem:(MakeTransferController *)makeTransferController;

- (void)viewToolbarClicked:(id)sender;

- (void)setDocumentModified:(BOOL)modified;


// Payees

- (IBAction)AddPayee:(id)sender;
- (IBAction)DeletePayee:(id)sender;

- (IBAction)AddCategory:(id)sender;
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

- (IBAction)showRecentTransactions:(id)sender;
- (IBAction)showAllTransactionsThisYear:(id)sender;
- (IBAction)showAllTransactions:(id)sender;

- (IBAction)ImportQIF:(id)sender;
- (void)importQIFConfirmed:(ImportQIFController *)importQIFController;

- (IBAction)ImportOFX:(id)sender;
- (void)importOFXFileWithController:(ImportOFXController*)controller reverseTransactions:(bool)reverse cleared:(bool)cleared ignoreExisting:(bool)ignoreExisting;
- (void)deleteImportOFXController;

- (IBAction)ExportOFX:(id)sender;
- (void)exportOFXFileWithController:(ExportOFXController*)controller xmlOFX:(bool)xml;
- (void)deleteExportOFXController;

- (IBAction)ExportQIF:(id)sender;

- (void)ScheduledSelectionDidChange:(NSNotification *)notification;
- (IBAction)updateScheduled:(id)sender;

- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem;

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

- (IBAction)showPreferencesWindow:(id)sender;

- (IBAction)gotoWebsite:(id)sender;
- (IBAction)reportBug:(id)sender;

@property (assign) IBOutlet NSWindow *window;

@end
