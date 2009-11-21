//
//  AccountInfoController.h
//  Stash
//
//  Created by Peter Pearson on 21/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "account.h"

@class StashAppDelegate;

@interface AccountInfoController : NSWindowController
{
	IBOutlet NSTextField *fAccountName;
	IBOutlet NSTextField *fInstitution;
	IBOutlet NSTextField *fNumber;
	IBOutlet NSTextField *fNote;
	
	IBOutlet NSPopUpButton *fAccountType;
	
	StashAppDelegate *fMainController;
	
	int nAccount;
	
	NSString *sAccountName;
	NSString *sInstitution;
	NSString *sNumber;
	NSString *sNote;
	AccountType eAccountType;
}

- (id)initWnd:(StashAppDelegate *)controller withAccount:(int)account name:(NSString*)name institution:(NSString*)institution
						number:(NSString*)number note:(NSString*)note type:(AccountType)type;

- (void)save:(id)sender;


@end
