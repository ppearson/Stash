//
//  AddAccountController.h
//  Stash
//
//  Created by Peter Pearson on 29/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class StashAppDelegate;


@interface AddAccountController : NSWindowController
{
	IBOutlet NSTextField *fAccountName, *fStartingBalance;
	
	IBOutlet NSPopUpButton *fAccountType;
	
	StashAppDelegate *fMainController;
	
	NSString *sAccountName, *sStartingBalance;
	int AccountType;
}

- (id)initWnd:(StashAppDelegate *)controller;

- (void)add:(id)sender;
- (void)cancel:(id)sender;

- (NSString *)accountName;
- (NSString *)startingBalance;
- (int)accountType;

@end
