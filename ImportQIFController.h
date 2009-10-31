//
//  QIFImportController.h
//  Stash
//
//  Created by Peter Pearson on 31/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class StashAppDelegate;

@interface ImportQIFController : NSWindowController
{
	IBOutlet NSTextField *fDescription;
	
	IBOutlet NSTextField *fSampleDate;	
	IBOutlet NSPopUpButton *fDateFormatPopup;
	IBOutlet NSTextField *fDateFormat;
	
	StashAppDelegate *fMainController;
	
	NSString *sDesc, *sFile, *sSampleDate;
	char cSeparator;
	int nDateFormat;	
}

- (int)dateFormat;
- (char)separator;
- (NSString*)file;

- (id)initWnd:(StashAppDelegate *)controller withFile:(NSString*)file sampleFormat:(NSString*)sFormat;

- (void)import:(id)sender;
- (void)cancel:(id)sender;

@end
