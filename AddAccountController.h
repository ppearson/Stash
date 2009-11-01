//
//  AddAccountController.h
//  Stash
//
//  Created by Peter Pearson on 29/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "account.h"

@class StashAppDelegate;

@interface AddAccountController : NSWindowController
{
	IBOutlet NSTextField *fAccountName;
	IBOutlet NSTextField *fStartingBalance;
	IBOutlet NSTextField *fInstitution;
	IBOutlet NSTextField *fNumber;
	IBOutlet NSTextField *fNote;
	
	IBOutlet NSPopUpButton *fAccountType;
	
	StashAppDelegate *fMainController;
	
	NSString *sAccountName;
	NSString *sStartingBalance;
	NSString *sInstitution;
	NSString *sNumber;
	NSString *sNote;
	AccountType eAccountType;
}

- (id)initWnd:(StashAppDelegate *)controller;

- (void)add:(id)sender;
- (void)cancel:(id)sender;

- (NSString *)accountName;
- (NSString *)startingBalance;
- (NSString *)institution;
- (NSString *)number;
- (NSString *)note;
- (AccountType)accountType;

@end
