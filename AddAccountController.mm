//
//  AddAccountController.m
//  Stash
//
//  Created by Peter Pearson on 29/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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
	[fStartingBalance setStringValue:@"0.0"];
	
	[fAccountType removeAllItems];
	
	[fAccountType addItemWithTitle:@"Cash"];
	[fAccountType addItemWithTitle:@"Checking"];
	[fAccountType addItemWithTitle:@"Credit Card"];
	[fAccountType addItemWithTitle:@"Investment"];
	[fAccountType addItemWithTitle:@"Asset"];
	[fAccountType addItemWithTitle:@"Liability"];
	[fAccountType addItemWithTitle:@"Other"];
	
	[fAccountType selectItemAtIndex:0];
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

