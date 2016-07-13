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

#import "ExportOFXController.h"

#import "StashAppDelegate.h"

@implementation ExportOFXController

- (id)initWithAccountData:(NSMutableArray *)availableAccounts file:(std::string)path
{
	if ((self = [super init]) != nil)
	{
		m_aAvailableAccounts = [[NSMutableArray alloc] init];
		
		[m_aAvailableAccounts addObjectsFromArray:availableAccounts];
		
		m_path = path;
	}
	return self;
}

- (void)dealloc
{
	[m_aAvailableAccounts release];
	
	[super dealloc];
}

- (void)showExportOFXWindow:(NSWindow *)window
{
	if (!exportOFXWindow)
	{
		[NSBundle loadNibNamed:@"ExportOFX" owner:self];
	}
	
	[fOFXVersion removeAllItems];
	[fOFXVersion addItemWithTitle:@"1.0 (SGML)"];
	[fOFXVersion addItemWithTitle:@"2.0 (XML)"];
	
	[NSApp beginSheet:exportOFXWindow modalForWindow:window modalDelegate:nil didEndSelector:nil contextInfo:nil];
}

- (IBAction)doExport:(id)sender
{
	int format = [fOFXVersion indexOfSelectedItem];
	
	bool bXML = false;
	
	if (format == 1)
		bXML = true;
	
	[[NSApp delegate] exportOFXFileWithController:self xmlOFX:bXML];
	
	[NSApp endSheet:exportOFXWindow];
	[exportOFXWindow orderOut:self];
}

- (IBAction)doCancel:(id)sender
{
	[[NSApp delegate] deleteImportOFXController];
	
	[NSApp endSheet:exportOFXWindow];
	[exportOFXWindow orderOut:self];
}

- (NSArray*)getAccounts
{
	return m_aAvailableAccounts;
}

- (NSString*)getFilename
{
	NSString *sFilename = [[NSString alloc] initWithUTF8String:m_path.c_str()];
	
	return sFilename;
}

- (id)tableView:(NSTableView *) aTableView objectValueForTableColumn:(NSTableColumn *) aTableColumn row:(NSInteger) rowIndex
{
	id result = @"";
	NSString *identifier = [aTableColumn identifier];
	
	NSMutableDictionary *oObject = [m_aAvailableAccounts objectAtIndex:rowIndex];
	
	result = [oObject valueForKey:identifier];
	
	return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [m_aAvailableAccounts count];
	
	return 0;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
	NSString *identifier = [tableColumn identifier];
	
	if ([identifier isEqualToString:@"export"])
	{
		if (rowIndex < 0)
			return;
		
		NSMutableDictionary *exportItem = [m_aAvailableAccounts objectAtIndex:rowIndex];
		
		if ([object boolValue] == YES)
		{
			[exportItem setObject:[NSNumber numberWithBool:YES] forKey:@"export"];
		}
		else
		{
			[exportItem setObject:[NSNumber numberWithBool:NO] forKey:@"export"];
		}		
	}
}


@end
