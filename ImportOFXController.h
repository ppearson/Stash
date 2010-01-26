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

#include <string>


@interface ImportOFXController : NSWindowController
{
	IBOutlet NSWindow *importOFXWindow;
	
	NSMutableArray *m_aExistingAccounts;
	NSMutableArray *m_aAvailableAccounts;

	IBOutlet NSTableView *fAvailableAccountsTable;
	IBOutlet id fImportThisAccount;
	IBOutlet NSMatrix *fRadioChoice;
	IBOutlet NSPopUpButton *fExistingAccounts;
	IBOutlet NSPopUpButton *fNewAccountType;
	IBOutlet NSTextField *fNewAccountName;
	IBOutlet id fReverseTransactionOrder;
	IBOutlet id fMarkAllAsCleared;
	IBOutlet id fIgnoreExistingTransactions;
	
	int m_lastSelectedAccount;
	std::string m_path;
}

- (id)initWithOFXData:(NSMutableArray *)availableAccounts existingAccounts:(NSMutableArray *)existingAccounts file:(std::string)path;
- (void)showImportOFXWindow:(NSWindow *)window reverse:(BOOL)reverseTransactions;

- (IBAction)importThisAccountToggled:(id)sender;
- (IBAction)existingAccountChanged:(id)sender;
- (IBAction)newAccountNameChanged:(id)sender;
- (IBAction)newAccountTypeChange:(id)sender;
- (IBAction)radioChoicesToggled:(id)sender;

- (IBAction)doImport:(id)sender;
- (IBAction)doCancel:(id)sender;

- (void)selectedAccountItemChanged:(NSNotification *)notification;

- (NSArray*)getSettings;
- (NSString*)getFilename;

@end
