//
//  QIFImportController.m
//  Stash
//
//  Created by Peter Pearson on 31/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#include "datetime.h"

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
		
		sDesc = [NSString stringWithFormat:@"Please select the settings you want to use to import the file \"%@\".", sFile];
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
	[fDateFormatPopup addItemWithTitle:@"mm/dd/YY"];
	[fDateFormatPopup addItemWithTitle:@"mm/dd/YYYY"];
		
	[fDateFormatPopup selectItemAtIndex:0];
}

- (void)dealloc
{
	[super dealloc];
}

- (void)import:(id)sender
{
	int nSelectedFormat = [fDateFormatPopup indexOfSelectedItem];
	
	switch (nSelectedFormat)
	{
		case 2:
		case 3:
			nDateFormat = US;
			break;
		case 0:
		case 1:
		default:
			nDateFormat = UK;			
	}
		
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
				 
- (NSString*)file
{
	[sFile retain];
	
	return sFile;
}

@end

@implementation ImportQIFController (Private)

- (void)confirmImport
{
	[self close];
	
	[fMainController importQIFConfirmed:self];	
}

@end

