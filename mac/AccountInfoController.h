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
	Account::Type eAccountType;
}

- (id)initWnd:(StashAppDelegate *)controller withAccount:(int)account name:(NSString*)name institution:(NSString*)institution
	   number:(NSString*)number note:(NSString*)note type:(Account::Type)type;

- (void)save:(id)sender;


@end
