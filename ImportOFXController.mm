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

#import "ImportOFXController.h"

@implementation ImportOFXController

- (id)initWithOFXData:(NSMutableArray *)availableAccounts existingAccounts:(NSMutableArray *)existingAccounts file:(std::string)path
{
	if ((self = [super init]) != nil)
	{
		m_aExistingAccounts = [[NSMutableArray alloc] init];
		m_aAvailableAccounts = [[NSMutableArray alloc] init];
		
		[m_aExistingAccounts addObjectsFromArray:existingAccounts];
		[m_aAvailableAccounts addObjectsFromArray:availableAccounts];
		
		m_lastSelectedAccount = -1;
		
		m_path = path;
	}
	return self;
}

- (void)dealloc
{
	[m_aExistingAccounts release];
	[m_aAvailableAccounts release];
	
	NSNotificationCenter *nc;
	nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self];
	
	[super dealloc];
}

- (void)showImportOFXWindow:(NSWindow *)window reverse:(BOOL)reverseTransactions
{
	if (!importOFXWindow)
	{
		[NSBundle loadNibNamed:@"ImportOFX" owner:self];
	}
	
	[fExistingAccounts removeAllItems];
	
	[fRadioChoice selectCellAtRow:1 column:0];
	
	NSButtonCell *existingChoiceButton = [fRadioChoice cellAtRow:0 column:0];
	if ([m_aExistingAccounts count] == 0)
	{
		[fExistingAccounts setEnabled:NO];
		[existingChoiceButton setEnabled:NO];
		
		[importOFXWindow makeFirstResponder:fNewAccountName];
	}
	else
	{
		[existingChoiceButton setEnabled:YES];
		[fExistingAccounts setEnabled:YES];
		[fNewAccountType setEnabled:YES];
		[fNewAccountName setEnabled:YES];
		
		NSString *sAccount = 0;
		for (sAccount in m_aExistingAccounts)
		{
			[fExistingAccounts addItemWithTitle:sAccount];
		}
	}
	
	[fNewAccountType removeAllItems];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Cash", "Account Type -> Cash")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Checking", "Account Type -> Checking")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Savings", "Account Type -> Savings")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Credit Card", "Account Type -> Credit Card")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Investment", "Account Type -> Investment")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Asset", "Account Type -> Asset")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Liability", "Account Type -> Liability")];
	[fNewAccountType addItemWithTitle:NSLocalizedString(@"Other", "Account Type -> Other")];
	
	if (reverseTransactions)
	{
		[fReverseTransactionOrder setState:NSOnState];
	}
	else
	{
		[fReverseTransactionOrder setState:NSOffState];
	}
	
	[fReverseTransactionOrder setToolTip:NSLocalizedString(@"Some Banks produce files with records in reverse order. This option reverses them on import.", "Import OFX Reverse Tooltip")];
	
	[fAvailableAccountsTable selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];

	[NSApp beginSheet:importOFXWindow modalForWindow:window modalDelegate:nil didEndSelector:nil contextInfo:nil];
}

- (void)awakeFromNib
{
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	
	[nc addObserver:self selector:@selector(selectedAccountItemChanged:) name:NSTableViewSelectionDidChangeNotification object:fAvailableAccountsTable];
	
}

- (void)selectedAccountItemChanged:(NSNotification *)notification
{
	// save new account name value
	if (m_lastSelectedAccount >= 0)
	{
		NSMutableDictionary *lastSelectedAccount = [m_aAvailableAccounts objectAtIndex:m_lastSelectedAccount];
		[lastSelectedAccount setObject:[fNewAccountName stringValue] forKey:@"newAccName"];		
	}
	
	NSInteger row = [fAvailableAccountsTable selectedRow];	
	NSMutableDictionary *accountItem = [m_aAvailableAccounts objectAtIndex:row];
	
	if (accountItem == nil)
		return;	
	
	BOOL bShouldImport = [[accountItem objectForKey:@"import"] boolValue];
	if (bShouldImport == YES)
		[fImportThisAccount setState:NSOnState];
	else
		[fImportThisAccount setState:NSOffState];
	
	int importType = [[accountItem objectForKey:@"importType"] intValue];
	[fRadioChoice selectCellAtRow:importType column:0];
	
	if (importType == 0)
	{
		[fExistingAccounts selectItemAtIndex:[[accountItem objectForKey:@"existingAccount"] intValue]];		
	}
	else
	{
		[fNewAccountType selectItemAtIndex:[[accountItem objectForKey:@"newAccType"] intValue]];
		[fNewAccountName setStringValue:[accountItem valueForKey:@"newAccName"]];
	}
	
	m_lastSelectedAccount = row;	
}

- (IBAction)importThisAccountToggled:(id)sender
{
	NSMutableDictionary *accountItem = [m_aAvailableAccounts objectAtIndex:[fAvailableAccountsTable selectedRow]];
	
	BOOL bImport = YES;
	
	if ([fImportThisAccount state] == NSOffState)
	{
		bImport = NO;
	}
	
	if (accountItem)
	{
		[accountItem setObject:[NSNumber numberWithBool:bImport] forKey:@"import"];
	}
	
	[fAvailableAccountsTable reloadData];
}

- (IBAction)existingAccountChanged:(id)sender
{
	NSMutableDictionary *accountItem = [m_aAvailableAccounts objectAtIndex:[fAvailableAccountsTable selectedRow]];
	
	if (accountItem)
	{
		[accountItem setObject:[NSNumber numberWithInt:[fExistingAccounts indexOfSelectedItem]] forKey:@"existingAccount"];		
	}
}
		 
- (IBAction)newAccountNameChanged:(id)sender
{
	
}

- (IBAction)newAccountTypeChange:(id)sender
{
	NSMutableDictionary *accountItem = [m_aAvailableAccounts objectAtIndex:[fAvailableAccountsTable selectedRow]];
	
	if (accountItem)
	{
		[accountItem setObject:[NSNumber numberWithInt:[fNewAccountType indexOfSelectedItem]] forKey:@"newAccType"];		
	}
}

- (IBAction)radioChoicesToggled:(id)sender
{	
	NSMatrix *radioMatrix = sender;
	NSButtonCell *radioButton = [radioMatrix selectedCell];
	NSInteger row;
	NSInteger column;
	[radioMatrix getRow:&row column:&column ofCell:radioButton];
	
	NSMutableDictionary *accountItem = [m_aAvailableAccounts objectAtIndex:[fAvailableAccountsTable selectedRow]];
	
	if (accountItem)
	{
		[accountItem setObject:[NSNumber numberWithInt:row] forKey:@"importType"];
	}
	
	if (row == 0)
	{
		[fExistingAccounts setEnabled:YES];
		
		[fNewAccountType setEnabled:NO];
		[fNewAccountName setEnabled:NO];
	}
	else
	{
		[fExistingAccounts setEnabled:NO];
		
		[fNewAccountType setEnabled:YES];
		[fNewAccountName setEnabled:YES];
		
		[importOFXWindow makeFirstResponder:fNewAccountName];
	}
}

- (IBAction)doImport:(id)sender
{
	bool bReverse = false;
	
	if ([fReverseTransactionOrder state] == NSOnState)
	{
		bReverse = true;
	}
	
	bool bMarkAsReconciled = false;
	
	if ([fMarkAllAsReconciled state] == NSOnState)
	{
		bMarkAsReconciled = true;
	}
	
	bool bIgnoreExisting = false;
	
	if ([fIgnoreExistingTransactions state] == NSOnState)
	{
		bIgnoreExisting = true;
	}
	
	if (m_lastSelectedAccount >= 0)
	{
		NSMutableDictionary *lastSelectedAccount = [m_aAvailableAccounts objectAtIndex:m_lastSelectedAccount];
		[lastSelectedAccount setObject:[fNewAccountName stringValue] forKey:@"newAccName"];		
	}
	
	[[NSApp delegate] importOFXFileWithController:self reverseTransactions:bReverse reconciled:bMarkAsReconciled ignoreExisting:bIgnoreExisting];
	
	[NSApp endSheet:importOFXWindow];
	[importOFXWindow orderOut:self];
}

- (IBAction)doCancel:(id)sender
{
	[[NSApp delegate] deleteImportOFXController];
	
	[NSApp endSheet:importOFXWindow];
	[importOFXWindow orderOut:self];
}

- (NSArray*)getSettings
{
	return m_aAvailableAccounts;
}

- (NSString*)getFilename
{
	NSString *sFilename = [[NSString alloc] initWithUTF8String:m_path.c_str()];
	
	return sFilename;
}

- (id)tableView:(NSTableView *) aTableView objectValueForTableColumn:(NSTableColumn *) aTableColumn row:(NSInteger) rowIndex
{
	id result = @"";
	NSString *identifier = [aTableColumn identifier];
	
	NSMutableDictionary *oObject = [m_aAvailableAccounts objectAtIndex:rowIndex];
	
	result = [oObject valueForKey:identifier];
	
	return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [m_aAvailableAccounts count];
	
	return 0;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
	NSString *identifier = [tableColumn identifier];
	
	if ([identifier isEqualToString:@"import"])
	{
		if (rowIndex < 0)
			return;
		
		NSMutableDictionary *exportItem = [m_aAvailableAccounts objectAtIndex:rowIndex];
		
		if ([object boolValue] == YES)
		{
			[exportItem setObject:[NSNumber numberWithBool:YES] forKey:@"import"];
		}
		else
		{
			[exportItem setObject:[NSNumber numberWithBool:NO] forKey:@"import"];
		}
		
		if (rowIndex == [fAvailableAccountsTable selectedRow])
		{
			if ([object boolValue] == YES)
				[fImportThisAccount setState:NSOnState];
			else
				[fImportThisAccount setState:NSOffState];
		}
	}
}

@end
