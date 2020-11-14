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

#include "date.h"

#import "ImportQIFController.h"

@interface ImportQIFController (Private)

- (void)confirmImport;

@end

@implementation ImportQIFController

- (id)initWnd:(StashAppDelegate *)controller withFile:(NSString*)file sampleFormat:(NSString*)sFormat
{
	if ((self = [super initWithWindowNibName:@"ImportQIF"]))
	{
		fMainController = controller;
		
		[file retain];
		sFile = file;
		
		[sFormat retain];
		sSampleDate = sFormat;
		
		sDesc = [NSString stringWithFormat:NSLocalizedString(@"Please select the settings you want to use to import the file \"%@\".", "QIF Import Window Text"), sFile];
	}
	
	return self;	
}

- (void)awakeFromNib
{
	[fDescription setStringValue:sDesc];
	
	[fSampleDate setStringValue:sSampleDate];
	
	[fDateFormatPopup removeAllItems];
	
	[fDateFormatPopup addItemWithTitle:@"dd/mm/YYYY"];
	[fDateFormatPopup addItemWithTitle:@"dd/mm/YY"];
	[fDateFormatPopup addItemWithTitle:@"dd-mm-YYYY"];
	[fDateFormatPopup addItemWithTitle:@"dd-mm-YY"];
	[fDateFormatPopup addItemWithTitle:@"mm/dd/YY"];
	[fDateFormatPopup addItemWithTitle:@"mm/dd/YYYY"];
	[fDateFormatPopup addItemWithTitle:@"mm-dd-YY"];
	[fDateFormatPopup addItemWithTitle:@"mm-dd-YYYY"];
		
	[fDateFormatPopup selectItemAtIndex:0];
}

- (void)dealloc
{
	[super dealloc];
}

- (void)import:(id)sender
{
	int nSelectedFormat = [fDateFormatPopup indexOfSelectedItem];
	
	// do the format
	
	nDateFormat = Date::UK;
	
	if (nSelectedFormat > 3)
		nDateFormat = Date::US;
		
	switch (nSelectedFormat)
	{
		case 0:
		case 1:
		case 4:
		case 5:
			cSeparator = '/';
			break;
		case 2:
		case 3:
		case 6:
		case 7:
			cSeparator = '-';			
	}
	
	if ([fSetAsCleared state] == NSOnState)
		bMarkCleared = YES;
	else
		bMarkCleared = NO;
		
	[self confirmImport];
}

- (void)cancel:(id)sender
{
	[[self window] performClose:sender];
}

- (int)dateFormat
{
	return nDateFormat;
}

- (char)separator
{
	return cSeparator;
}
				 
- (NSString*)file
{
	[sFile retain];
	
	return sFile;
}

- (BOOL)markAsCleared
{
	return bMarkCleared;
}

@end

@implementation ImportQIFController (Private)

- (void)confirmImport
{
	[self close];
	
	[fMainController importQIFConfirmed:self];	
}

@end

