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
	
	AccountType = 0;
	
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

- (int)accountType
{
	return AccountType;
}

@end

@implementation AddAccountController (Private)

- (void)confirmAdd
{
	[self close];
	
	[fMainController addAccountConfirmed:self];	
}

@end

