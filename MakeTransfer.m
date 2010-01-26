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

#import "MakeTransfer.h"

@implementation MakeTransfer

- (id)initWithAccounts:(NSMutableArray *)accounts categories:(NSMutableArray *)categories
{
	if ((self = [super init]) != nil)
	{
		m_aAccounts = [[NSMutableArray alloc] init];
		m_aCategories = [[NSMutableArray alloc] init];
		
		[m_aAccounts addObjectsFromArray:accounts];
		[m_aCategories addObjectsFromArray:categories];
	}
	return self;
}

- (void)dealloc
{
	[m_aAccounts release];
	[m_aCategories release];
	
	[super dealloc];
}

- (void)makeTransfer:(NSWindow *)window initialAccount:(int)initAccount
{
	if (!makeTransferWindow)
	{
		[NSBundle loadNibNamed:@"MakeTransfer" owner:self];
	}
	
	[fromAccount removeAllItems];
	[toAccount removeAllItems];
	
	NSString *sAccount = 0;
	for (sAccount in m_aAccounts)
	{
		[fromAccount addItemWithTitle:sAccount];
		[toAccount addItemWithTitle:sAccount];
	}
	
	[fromAccount selectItemAtIndex:initAccount];
	[toAccount selectItemAtIndex:0];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	NSNumber *number = [NSNumber numberWithInt:0];
	m_amount = [numberFormatter stringFromNumber:number];
	
	[amount setStringValue:m_amount];
	
	[numberFormatter release];
	
	[category removeAllItems];
	
	[category setStringValue:@""];
	[description setStringValue:@""];
	
	NSString *sCategory = 0;
	for (sCategory in m_aCategories)
	{
		[category addItemWithObjectValue:sCategory];
	}
	
	NSDate *date1 = [NSDate date];
	[dateCntl setDateValue:date1];
	
	[makeTransferWindow makeFirstResponder:fromAccount];
	[NSApp beginSheet:makeTransferWindow modalForWindow:window modalDelegate:nil didEndSelector:nil contextInfo:nil];
}

- (IBAction)doMakeTransfer:(id)sender
{
	m_fromAccount = [fromAccount indexOfSelectedItem];
	m_toAccount = [toAccount indexOfSelectedItem];
	m_amount = [amount stringValue];
	m_category = [category stringValue];
	m_description = [description stringValue];
	m_date = [dateCntl dateValue];
	
	if ([setAsCleared state] == NSOnState)
		m_makeCleared = YES;
	else
		m_makeCleared = NO;
	
	if (m_fromAccount == m_toAccount)
	{
		NSRunAlertPanel(NSLocalizedString(@"Invalid account selection", "Make Transfer -> Invalid account selection Warning Title"),
						NSLocalizedString(@"You must select different From and To accounts in order to make a Transfer.", "Make Transfer -> Invalid account selection Warning Message"),
						NSLocalizedString(@"OK", "OK Button"), nil, nil);
		return;
	}
	
	[[NSApp delegate] makeTransferItem:self];
	
	[NSApp endSheet:makeTransferWindow];
	[makeTransferWindow orderOut:self];
}

- (IBAction)doCancel:(id)sender
{
	[NSApp endSheet:makeTransferWindow];
	[makeTransferWindow orderOut:self];
}

- (int)fromAccount
{
	return m_fromAccount;
}

- (int)toAccount
{
	return m_toAccount;
}

- (NSString*)amount
{
	return m_amount;
}

- (NSString*)category
{
	return m_category;
}

- (NSString*)description
{
	return m_description;
}

- (NSDate*)date
{
	return m_date;
}

- (BOOL)makeCleared
{
	return m_makeCleared;
}

@end
