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

#include "account.h"
#include "document.h"
#include "transaction.h"

#import "PieChartView.h"
#import "AreaChartView.h"
#import "OverviewChartView.h"

@class StashAppDelegate;

@interface GraphController : NSViewController
{
	StashAppDelegate *fMainController;
	
	IBOutlet NSSegmentedControl* viewingPeriodSegmentControl;
	IBOutlet NSTabView *graphViewType;
	IBOutlet PieChartView *pieChartView;
	IBOutlet AreaChartView *areaChartView;
	IBOutlet OverviewChartView *overviewChartView;
	IBOutlet NSPopUpButton *graphAccount;
	IBOutlet NSPopUpButton *graphType;
	IBOutlet id graphStartDateCntrl;
	IBOutlet id graphEndDateCntrl;
	IBOutlet id graphIgnoreTransfers;
	IBOutlet NSPopUpButton *graphItemTypes;
	IBOutlet NSTableView *graphItemsTableView;
	
	NSMutableArray *m_aGraphItems;
	
	Document *m_pDocument;
	Graph *m_pGraph;
	int m_nGraphDateSegment;
}

+ (GraphController*)sharedInterface;

-(id)retain;
- (NSUInteger)retainCount;
-(void)release;
-(id)autorelease;

-(id)init;

- (void)setMainController:(StashAppDelegate *)controller;

- (void)showGraph:(Graph*)graph;

- (void)buildGraph:(int)account startDate:(NSDate*)startDate endDate:(NSDate*)endDate type:(GraphType)type ignoreTransfers:(bool)ignoreTransfers;

- (IBAction)redrawGraph:(id)sender;
- (IBAction)dateBarClicked:(id)sender;
- (IBAction)graphDatesManuallyChanged:(id)sender;

- (IBAction)addGraphItem:(id)sender;
- (IBAction)addSelectedGraphItem:(NSString *)item;
- (IBAction)deleteGraphItem:(id)sender;

- (IBAction)updateGraph:(id)sender;

NSDate *convertToNSDate(MonthYear &date);

- (void)handleGraphSettingsUpdate:(NSNotification *)note;

- (void)cleanUp;

- (void)setDocumentModified:(BOOL)modified;

@end
