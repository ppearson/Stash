//
//  AccountInfoController.m
//  Stash
//
//  Created by Peter Pearson on 21/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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
