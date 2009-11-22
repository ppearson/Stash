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

#import "AccountInfoController.h"

@implementation AccountInfoController

- (id)initWnd:(StashAppDelegate *)controller withAccount:(int)account name:(NSString*)name institution:(NSString*)institution
	   number:(NSString*)number note:(NSString*)note type:(AccountType)type
{
	if ((self = [super initWithWindowNibName:@"AccountInfo"]))
	{
		fMainController = controller;
		
		nAccount = account;
		
		[name retain];
		sAccountName = name;
		
		[institution retain];
		sInstitution = institution;
		
		[number retain];
		sNumber = number;
		
		[note retain];
		sNote = note;
		
		eAccountType = type;
	}
	
	return self;	
}

- (void)awakeFromNib
{
	[fAccountType removeAllItems];
	
	[fAccountType addItemWithTitle:@"Cash"];
	[fAccountType addItemWithTitle:@"Checking"];
	[fAccountType addItemWithTitle:@"Savings"];
	[fAccountType addItemWithTitle:@"Credit Card"];
	[fAccountType addItemWithTitle:@"Investment"];
	[fAccountType addItemWithTitle:@"Asset"];
	[fAccountType addItemWithTitle:@"Liability"];
	[fAccountType addItemWithTitle:@"Other"];
	
	[fAccountType selectItemAtIndex:eAccountType];
	
	[fAccountName setStringValue:sAccountName];
	[fInstitution setStringValue:sInstitution];
	[fNumber setStringValue:sNumber];
	[fNote setStringValue:sNote];	
}

- (void)dealloc
{
	[super dealloc];
}

- (void)save:(id)sender
{
	sAccountName = [fAccountName stringValue];
	
	sInstitution = [fInstitution stringValue];
	sNumber = [fNumber stringValue];
	sNote = [fNote stringValue];
	
	int nSelectedType = [fAccountType indexOfSelectedItem];
	
	eAccountType = static_cast<AccountType>(nSelectedType);
	
	[fMainController updateAccountInfo:nAccount name:sAccountName institution:sInstitution number:sNumber
									note:sNote type:eAccountType];
	
	[[self window] performClose:sender];
}

@end
