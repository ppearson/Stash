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

#import <Cocoa/Cocoa.h>

#include <string>

@interface ExportOFXController : NSWindowController
{
	IBOutlet NSWindow *exportOFXWindow;
	
	NSMutableArray *m_aAvailableAccounts;
	
	IBOutlet NSTableView *fAvailableAccountsTable;
	IBOutlet NSPopUpButton *fOFXVersion;
	
	std::string m_path;
}

- (id)initWithAccountData:(NSMutableArray *)availableAccounts file:(std::string)path;
- (void)showExportOFXWindow:(NSWindow *)window;

- (IBAction)doExport:(id)sender;
- (IBAction)doCancel:(id)sender;

- (NSArray*)getAccounts;
- (NSString*)getFilename;

@end
