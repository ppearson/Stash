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

#import "AddAccountController.h"

@interface AddAccountController (Private)

- (void)confirmAdd;

@end

@implementation AddAccountController

- (id)initWnd:(StashAppDelegate *)controller
{
	if ((self = [super initWithWindowNibName:@"AddAccount"]))
	{
		fMainController = controller;
	}
	
	return self;	
}

- (void)awakeFromNib
{
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	NSNumber *number = [NSNumber numberWithInt:0];
	sStartingBalance = [numberFormatter stringFromNumber:number];
	
	[fStartingBalance setStringValue:sStartingBalance];
	
	[fAccountType removeAllItems];
	
	[fAccountType addItemWithTitle:@"Cash"];
	[fAccountType addItemWithTitle:@"Checking"];
	[fAccountType addItemWithTitle:@"Savings"];
	[fAccountType addItemWithTitle:@"Credit Card"];
	[fAccountType addItemWithTitle:@"Investment"];
	[fAccountType addItemWithTitle:@"Asset"];
	[fAccountType addItemWithTitle:@"Liability"];
	[fAccountType addItemWithTitle:@"Other"];
	
	[fAccountType selectItemAtIndex:0];
	
	[numberFormatter release];
}

- (void)dealloc
{
//	[sAccountName release];
	
	[super dealloc];
}

- (void)add:(id)sender
{
	sAccountName = [fAccountName stringValue];
	sStartingBalance = [fStartingBalance stringValue];
	
	sInstitution = [fInstitution stringValue];
	sNumber = [fNumber stringValue];
	sNote = [fNote stringValue];
	
	int nSelectedType = [fAccountType indexOfSelectedItem];
	
	eAccountType = static_cast<AccountType>(nSelectedType);
	
	[self confirmAdd];
}

-(void)cancel:(id)sender
{
	[[self window] performClose:sender];	
}

- (NSString *)accountName
{
	return sAccountName;
}

- (NSString *)startingBalance
{
	return sStartingBalance;
}

- (NSString *)institution
{
	return sInstitution;
}

- (NSString *)number
{
	return sNumber;
}

- (NSString *)note
{
	return sNote;
}

- (AccountType)accountType
{
	return eAccountType;
}

@end

@implementation AddAccountController (Private)

- (void)confirmAdd
{
	[self close];
	
	[fMainController addAccountConfirmed:self];	
}

@end

