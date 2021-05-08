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

#include "analysis.h"

#import "GraphController.h"
#import "TransactionsController.h"
#import "NSDateEx.h"
#import "ValueFormatter.h"

@implementation GraphController

static GraphController *gSharedInterface = nil;

+ (GraphController*)sharedInterface
{
	@synchronized(self)
	{
		if (gSharedInterface == nil)
		{
			gSharedInterface = [[super allocWithZone:NULL] init];
		}
	}
	return gSharedInterface;
}

+ (id)allocWithZone:(NSZone *)zone
{
	return [[self sharedInterface] retain];
}

- (id)copyWithZone:(NSZone *)zone
{
	return self;
}

- (id)retain
{
	return self;
}

- (NSUInteger)retainCount
{
	return NSUIntegerMax;
}

- (void)release
{
	// do nothing
}

- (id)autorelease
{
	return self;
}

- (id)init
{
	self = [super init];
	
	return self;
}

- (void)dealloc
{	
	[super dealloc];
}

- (void)setMainController:(StashAppDelegate *)controller
{
	fMainController = controller;
}

- (void)awakeFromNib
{
	m_aGraphItems = [[NSMutableArray alloc] init];
	
	[graphType removeAllItems];
	
	[graphType addItemWithTitle:NSLocalizedString(@"Expense Categories", "Graph Pane -> Type -> Expense Categories")];
	[graphType addItemWithTitle:NSLocalizedString(@"Expense Payees", "Graph Pane -> Type -> Expense Payees")];
	[graphType addItemWithTitle:NSLocalizedString(@"Deposit Categories", "Graph Pane -> Type -> Deposit Categories")];
	[graphType addItemWithTitle:NSLocalizedString(@"Deposit Payees", "Graph Pane -> Type -> Deposit Payees")];
	
	[graphItemTypes removeAllItems];
	
	[graphItemTypes addItemWithTitle:NSLocalizedString(@"All Items", "Graph Pane -> Items Type -> All Items")];
	[graphItemTypes addItemWithTitle:NSLocalizedString(@"All Items excluding:", "Graph Pane -> Items Type -> All Items excluding:")];
	[graphItemTypes addItemWithTitle:NSLocalizedString(@"Only Specified:", "Graph Pane -> Items Type -> Only Specified:")];
	
	[graphItemsTableView setDelegate:self];
}

- (void)cleanUp
{
	[m_aGraphItems release];
}

- (void)showGraph:(Graph*)graph
{
	if (!graph)
		return;
	
	m_pDocument = Document::getInstance();
	
	m_pGraph = graph;
	
	// Update the list of Accounts
	
	[graphAccount removeAllItems];
	
	std::vector<Account>::const_iterator itAccount = m_pDocument->AccountBegin();
	
	for (; itAccount != m_pDocument->AccountEnd(); ++itAccount)
	{
		std::string strName = itAccount->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		[graphAccount addItemWithTitle:sName];
		
		[sName release];
	}
	
	int nAccount = m_pGraph->getAccount();
	
	[graphAccount selectItemAtIndex:nAccount];
	
	NSDate *nsStartDate = convertToNSDate(const_cast<Date&>(m_pGraph->getStartDate()));
	[graphStartDateCntrl setDateValue:nsStartDate];
	
	NSDate *nsEndDate = convertToNSDate(const_cast<Date&>(m_pGraph->getEndDate()));
	[graphEndDateCntrl setDateValue:nsEndDate];
	
	Graph::Type eType = m_pGraph->getType();
	
	[graphType selectItemAtIndex:eType];
	
	if (m_pGraph->getIgnoreTransfers())
	{
		[graphIgnoreTransfers setState:NSOnState];
	}
	else
	{
		[graphIgnoreTransfers setState:NSOffState];
	}
	
	Graph::ItemsType eItemsType = m_pGraph->getItemsType();
	[graphItemTypes selectItemAtIndex:eItemsType];
	
	[m_aGraphItems removeAllObjects];
	
	std::set<std::string> &aItems = m_pGraph->getItems();
	
	for (std::set<std::string>::iterator itItems = aItems.begin(); itItems != aItems.end(); ++itItems)
	{
		std::string strItem = (*itItems);
		
		NSString *sItem = [[NSString alloc] initWithUTF8String:strItem.c_str()];
		
		[m_aGraphItems addObject:sItem];		
	}
	
	[graphItemsTableView reloadData];
	
	Graph::DateType eDateType = m_pGraph->getDateType();
	
	int nDatePeriodSegment = 0;
	
	switch (eDateType)
	{
		case Graph::DateWeek:
			nDatePeriodSegment = 1;
			break;
		case Graph::DateMonth:
			nDatePeriodSegment = 2;
			break;
		case Graph::DateYear:
			nDatePeriodSegment = 3;
			break;
		default:
			nDatePeriodSegment = 4;
			break;
	}
	
	m_nGraphDateSegment = nDatePeriodSegment;
	[viewingPeriodSegmentControl setSelectedSegment:nDatePeriodSegment];
	
	if (nDatePeriodSegment == 4)
	{
		[viewingPeriodSegmentControl setEnabled:NO forSegment:0];
		[viewingPeriodSegmentControl setEnabled:NO forSegment:5];
	}
	else
	{
		[viewingPeriodSegmentControl setEnabled:YES forSegment:0];
		[viewingPeriodSegmentControl setEnabled:YES forSegment:5];
	}
	
	Graph::ViewType eViewType = m_pGraph->getViewType();
	
	[graphViewType selectTabViewItemAtIndex:eViewType];
	
	// for some reason, the above sets/selects don't always update the controls properly
	[graphAccount setNeedsDisplay:YES];
	[graphStartDateCntrl setNeedsDisplay:YES];
	[graphEndDateCntrl setNeedsDisplay:YES];
	
	[self buildGraph:nAccount startDate:nsStartDate endDate:nsEndDate type:eType ignoreTransfers:m_pGraph->getIgnoreTransfers()];
}

- (void)buildGraph:(int)account startDate:(NSDate*)startDate endDate:(NSDate*)endDate type:(Graph::Type)type ignoreTransfers:(bool)ignoreTransfers
{
	if (!m_pGraph)
		return;
	
	if (account < 0)
		return;
	
	Account* pAccount = &m_pDocument->getAccount(account);
	
	NSCalendarDate *nsCalStartDate = [startDate dateWithCalendarFormat:0 timeZone:0];
	
	int nStartYear = [nsCalStartDate yearOfCommonEra];
	int nStartMonth = [nsCalStartDate monthOfYear];
	int nStartDay = [nsCalStartDate dayOfMonth];
	
	Date mainStartDate(nStartDay, nStartMonth, nStartYear);
	
	NSCalendarDate *nsCalEndDate = [endDate dateWithCalendarFormat:0 timeZone:0];
	
	int nEndYear = [nsCalEndDate yearOfCommonEra];
	int nEndMonth = [nsCalEndDate monthOfYear];
	int nEndDay = [nsCalEndDate dayOfMonth];
	
	Date mainEndDate(nEndDay, nEndMonth, nEndYear);
	
	// Pie Chart Items
	
	int pieSmallerThanValue = -1;
	std::string pieGroupSmallerName = "";
	
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"PieChartGroupSmallerItems"] == YES)
	{
		pieSmallerThanValue = [[NSUserDefaults standardUserDefaults] integerForKey:@"PieChartGroupSmallerItemsSize"];
		NSString *sGroupOtherName = [[NSUserDefaults standardUserDefaults] valueForKey:@"PieChartGroupSmallerItemsName"];
		
		pieGroupSmallerName = [sGroupOtherName cStringUsingEncoding:NSUTF8StringEncoding]; 
	}
	
	PieChartCriteria::PieChartSort ePieChartSort = static_cast<PieChartCriteria::PieChartSort>([[NSUserDefaults standardUserDefaults] integerForKey:@"PieChartSortType"]);
	
	PieChartCriteria pieCriteria(pAccount, mainStartDate, mainEndDate, ignoreTransfers, pieSmallerThanValue, pieGroupSmallerName, ePieChartSort);
	
	PieChartResults pieResults;
	
	//
	
	int nItemsType = [graphItemTypes indexOfSelectedItem];
	Graph::ItemsType eItemsType = static_cast<Graph::ItemsType>(nItemsType);
	
	pieCriteria.m_itemsType = eItemsType;
	
	std::set<std::string> &aItems = pieCriteria.m_aItems;
	
	for (NSString *sItem in m_aGraphItems)
	{
		std::string strItem = [sItem cStringUsingEncoding:NSUTF8StringEncoding];
		
		aItems.insert(strItem);
	}
	
	if (type == Graph::ExpenseCategories)
		buildPieChartItems(pieCriteria, pieResults, true, true);
	else if (type == Graph::ExpensePayees)
		buildPieChartItems(pieCriteria, pieResults, true, false);
	else if (type == Graph::DepositCategories)
		buildPieChartItems(pieCriteria, pieResults, false, true);
	else if (type == Graph::DepositPayees)
		buildPieChartItems(pieCriteria, pieResults, false, false);
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	NSMutableArray *aPieItems = [[NSMutableArray alloc] init];
	
	std::vector<PieChartItem>::const_iterator itPie = pieResults.m_aValues.begin();
	
	double startAngle = 0.0;
	
	for (; itPie != pieResults.m_aValues.end(); ++itPie)
	{
		NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
		
		const std::string& strTitle = (*itPie).getTitle();
		fixed amount = (*itPie).getAmount();
		double angle = (*itPie).getAngle();
		
		NSString *sTitle = [[NSString alloc] initWithUTF8String:strTitle.c_str()];
		
		NSNumber *nSAmount = [NSNumber numberWithDouble:amount.ToDouble()];
		NSString *sSAmount = [[valueFormatter currencyStringFromNumber:nSAmount] retain];
		
		[dict setValue:[NSNumber numberWithDouble:angle] forKey:@"angle"];
		
		[dict setValue:[NSNumber numberWithDouble:startAngle] forKey:@"startangle"];
		
		startAngle += angle;
		
		[dict setValue:[NSNumber numberWithDouble:startAngle] forKey:@"endangle"];		 
		
		[dict setValue:sTitle forKey:@"title"];
		[dict setValue:sSAmount forKey:@"amount"];
		
		[dict setValue:nSAmount forKey:@"numamount"];
		
		[aPieItems addObject:dict];		
	}
	
	NSString *sTotal = [[valueFormatter currencyStringFromFixed:pieResults.m_overallTotal] retain];
	
	[pieChartView setTotal:sTotal];
	[pieChartView setData:aPieItems];
	
	// Area Chart Items
	int areaSmallerThanValue = -1;
	std::string areaGroupSmallerName = "";
	
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"AreaChartGroupSmallerItems"] == YES)
	{
		areaSmallerThanValue = [[NSUserDefaults standardUserDefaults] integerForKey:@"AreaChartGroupSmallerItemsSize"];
		NSString *sGroupOtherName = [[NSUserDefaults standardUserDefaults] valueForKey:@"AreaChartGroupSmallerItemsName"];
		
		areaGroupSmallerName = [sGroupOtherName cStringUsingEncoding:NSUTF8StringEncoding]; 
	}
	
	AreaChartCriteria areaCriteria(pAccount, mainStartDate, mainEndDate, ignoreTransfers,
								   areaSmallerThanValue, areaGroupSmallerName);
	
	areaCriteria.m_itemsType = eItemsType;
	areaCriteria.m_aItems = pieCriteria.m_aItems;
	
	AreaChartResults areaResults;
	
	if (type == Graph::ExpenseCategories)
		buildAreaChartItems(areaCriteria, areaResults, true, true);
	else if (type == Graph::ExpensePayees)
		buildAreaChartItems(areaCriteria, areaResults, true, false);
	else if (type == Graph::DepositCategories)
		buildAreaChartItems(areaCriteria, areaResults, false, true);
	else if (type == Graph::DepositPayees)
		buildAreaChartItems(areaCriteria, areaResults, false, false);
	
	NSMutableArray *aAreaItems = [[NSMutableArray alloc] init];
	
	std::vector<AreaChartItem>::iterator itArea = areaResults.m_aValues.begin();
	
	for (; itArea != areaResults.m_aValues.end(); ++itArea)
	{
		NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
		
		std::string strTitle = (*itArea).getTitle();
		NSString *sTitle = [[NSString alloc] initWithUTF8String:strTitle.c_str()];
		
		[dict setValue:sTitle forKey:@"title"];
		
		NSMutableArray *aAmounts = [[NSMutableArray alloc] init];
		
		int nNumItems = (*itArea).getNumItems();
		
		for (int i = 0; i < nNumItems; i++)
		{
			fixed amount = (*itArea).getItemAmount(i);
			NSNumber *nsAmount = [NSNumber numberWithDouble:amount.ToDouble()];
			
			[aAmounts addObject:nsAmount];			
		}
		
		[dict setValue:aAmounts forKey:@"amounts"];
		
		[aAreaItems addObject:dict];		
	}
	
	NSMutableArray *aDates = [[NSMutableArray alloc] init];
	
	std::vector<MonthYear>::iterator itDate = areaResults.m_aDates.begin();
	
	unsigned int nLongestDateLength = 0;
	unsigned int nLongestDateIndex = -1;
	
	int nIndex = 0;
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateFormat:@"MMM\nyyyy"];
	
	for (; itDate != areaResults.m_aDates.end(); ++itDate, nIndex++)
	{
		NSDate *thisDate = convertToNSDate(*itDate);
		
		NSString *sDate = [[dateFormatter stringFromDate:thisDate] retain];
		
		if ([sDate length] > nLongestDateLength)
		{
			nLongestDateLength = [sDate length];
			nLongestDateIndex = nIndex;
		}
		
		[aDates addObject:sDate];
	}
	
	if (nLongestDateIndex >= 0)
	{
		[areaChartView setLongestDate:[aDates objectAtIndex:nLongestDateIndex]];
		[areaChartView setDates:aDates];
		[areaChartView setMaxValue:areaResults.m_overallMax.ToDouble()];
		[areaChartView setData:aAreaItems];
	}
	else // redraw anyway, so view displays lack of data message
	{
		[areaChartView setDates:aDates];
		[areaChartView setMaxValue:areaResults.m_overallMax.ToDouble()];
		[areaChartView setData:aAreaItems];
	}
	
	// Overview chart
	
	fixed overallMax = 0.0;
	
	OverviewChartCriteria overviewCriteria(pAccount, mainStartDate, mainEndDate, ignoreTransfers, overallMax);
	std::vector<OverviewChartItem> aOverviewChartItems;
	
	buildOverviewChartItems(overviewCriteria, aOverviewChartItems);
	
	NSMutableArray *aOverviewItems = [[NSMutableArray alloc] init];
	
	std::vector<OverviewChartItem>::iterator itOverview = aOverviewChartItems.begin();
	
	for (; itOverview != aOverviewChartItems.end(); ++itOverview)
	{
		NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
		
		fixed fIncome = (*itOverview).m_income;
		NSNumber *nsIncome = [NSNumber numberWithDouble:fIncome.ToDouble()];
		[dict setValue:nsIncome forKey:@"income"];
		
		fixed fOutgoings = (*itOverview).m_outgoings;
		NSNumber *nsOutgoings = [NSNumber numberWithDouble:fOutgoings.ToDouble()];
		[dict setValue:nsOutgoings forKey:@"outgoings"];
		
		NSDate *thisDate = convertToNSDate((*itOverview).m_date);
		NSString *sDate = [[dateFormatter stringFromDate:thisDate] retain];
		
		[dict setValue:sDate forKey:@"date"];
		
		[aOverviewItems addObject:dict];		
	}
	
	[dateFormatter release];
	
	[overviewChartView setData:aOverviewItems];
	[overviewChartView setMaxValue:overallMax.ToDouble()];
}

- (IBAction)redrawGraph:(id)sender
{
	bool bIgnoreTransfers = false;
	
	if ([graphIgnoreTransfers state] == NSOnState)
		bIgnoreTransfers = true;
	
	int nAccount = [graphAccount indexOfSelectedItem];
	
	NSDate *nsStartDate = [graphStartDateCntrl dateValue];
	NSDate *nsEndDate = [graphEndDateCntrl dateValue];
	
	int nType = [graphType indexOfSelectedItem];
	Graph::Type eType = static_cast<Graph::Type>(nType);
	
	[self buildGraph:nAccount startDate:nsStartDate endDate:nsEndDate type:eType ignoreTransfers:bIgnoreTransfers];
}

- (IBAction)graphDatesManuallyChanged:(id)sender
{
	[viewingPeriodSegmentControl setEnabled:NO forSegment:0];
	[viewingPeriodSegmentControl setEnabled:NO forSegment:5];
	
	[viewingPeriodSegmentControl setSelectedSegment:4];
	
	[self redrawGraph:self];
}

- (IBAction)addGraphItem:(id)sender
{
	int count = [m_aGraphItems count];
	
	NSString *sNewItem = NSLocalizedString(@"New Item", "New Graph Item Name");
	
	[m_aGraphItems addObject:sNewItem];
	
	[graphItemsTableView reloadData];
	
	[graphItemsTableView editColumn:0 row:count withEvent:nil select:YES];	
}

- (IBAction)addSelectedGraphItem:(NSString *)item
{
	// if this is the first item added and we're in view all mode, change to View all excluding mode
	if ([m_aGraphItems count] == 0)
	{
		int nItemsType = [graphItemTypes indexOfSelectedItem];
		Graph::ItemsType eItemsType = static_cast<Graph::ItemsType>(nItemsType);
		
		if (eItemsType == Graph::AllItems)
			eItemsType = Graph::AllItemsExceptSpecified;
		
		[graphItemTypes selectItemAtIndex:eItemsType];
	}
	
	[m_aGraphItems addObject:item];
	
	[graphItemsTableView reloadData];
	[self redrawGraph:self];
}

- (IBAction)deleteGraphItem:(id)sender
{
	NSIndexSet *rows = [graphItemsTableView selectedRowIndexes];
	
	int nCount = [rows count];
	
	if (nCount > 0)
	{
		NSInteger row = [rows lastIndex];
		
		while (row != NSNotFound)
		{
			[m_aGraphItems removeObjectAtIndex:row];
			
			row = [rows indexLessThanIndex:row];
		}
		
		[graphItemsTableView reloadData];
		[graphItemsTableView deselectAll:self];
		[self redrawGraph:self];
	}
}

- (IBAction)dateBarClicked:(id)sender
{
	if (![sender isKindOfClass:[NSSegmentedControl class]])
		return;
	
	static int nCurrentSegment = m_nGraphDateSegment;
	
	int nSegment = [sender selectedSegment];
	
	if (nSegment == 0 || nSegment == 5)
	{
		int diff = 1;
		
		if (nSegment == 0)
			diff = -1;
		
		NSDate *nsStartDate = [graphStartDateCntrl dateValue];
		NSDate *nsEndDate = [graphEndDateCntrl dateValue];
		
		switch(nCurrentSegment)
		{
			case 1:
				nsStartDate = [nsStartDate addWeeks:diff];
				nsEndDate = [nsEndDate addWeeks:diff];
				break;
			case 2:
				nsStartDate = [nsStartDate addMonths:diff];
				nsStartDate = [nsStartDate firstDayOfMonth];
				nsEndDate = [nsStartDate lastDayOfMonth];
				break;
			case 3:
				nsStartDate = [nsStartDate addYears:diff];
				nsStartDate = [nsStartDate firstDayOfYear];
				nsEndDate = [nsStartDate lastDayOfYear];
				break;
		}
		
		[graphStartDateCntrl setDateValue:nsStartDate];
		[graphEndDateCntrl setDateValue:nsEndDate];
		
		[viewingPeriodSegmentControl setSelectedSegment:nCurrentSegment];
	}
	else
	{
		nCurrentSegment = [viewingPeriodSegmentControl selectedSegment];
		
		NSDate *nsStartDate = [graphStartDateCntrl dateValue];
		NSDate *nsEndDate = [graphEndDateCntrl dateValue];
		
		switch(nCurrentSegment)
		{
			case 1:
				nsEndDate = [nsStartDate addWeeks:1];
				break;
			case 2:
				nsStartDate = [nsStartDate firstDayOfMonth];
				nsEndDate = [nsStartDate lastDayOfMonth];
				break;
			case 3:
				nsStartDate = [nsStartDate firstDayOfYear];
				nsEndDate = [nsStartDate lastDayOfYear];
				break;
		}
		
		// enable/disable left/right arrows based on whether User defined segment is selected
		if (nCurrentSegment == 4)
		{
			[viewingPeriodSegmentControl setEnabled:NO forSegment:0];
			[viewingPeriodSegmentControl setEnabled:NO forSegment:5];
		}
		else
		{
			[viewingPeriodSegmentControl setEnabled:YES forSegment:0];
			[viewingPeriodSegmentControl setEnabled:YES forSegment:5];
		}
		
		[graphStartDateCntrl setDateValue:nsStartDate];
		[graphEndDateCntrl setDateValue:nsEndDate];
	}
	
	[self redrawGraph:self];
}

- (IBAction)updateGraph:(id)sender
{
	if (!m_pGraph)
		return;
	
	int nAccount = [graphAccount indexOfSelectedItem];
	
	NSDate *nsStartDate = [graphStartDateCntrl dateValue];
	NSCalendarDate *nsCalStartDate = [nsStartDate dateWithCalendarFormat:0 timeZone:0];
	
	int nStartYear = [nsCalStartDate yearOfCommonEra];
	int nStartMonth = [nsCalStartDate monthOfYear];
	int nStartDay = [nsCalStartDate dayOfMonth];
	
	Date startDate(nStartDay, nStartMonth, nStartYear);
	
	NSDate *nsEndDate = [graphEndDateCntrl dateValue];
	NSCalendarDate *nsCalEndDate = [nsEndDate dateWithCalendarFormat:0 timeZone:0];
	
	int nEndYear = [nsCalEndDate yearOfCommonEra];
	int nEndMonth = [nsCalEndDate monthOfYear];
	int nEndDay = [nsCalEndDate dayOfMonth];
	
	Date endDate(nEndDay, nEndMonth, nEndYear);
	
	int nType = [graphType indexOfSelectedItem];
	Graph::Type eType = static_cast<Graph::Type>(nType);
	
	bool bIgnoreTransfers = false;
	
	if ([graphIgnoreTransfers state] == NSOnState)
		bIgnoreTransfers = true;
	
	int nDatePeriodSegment = [viewingPeriodSegmentControl selectedSegment];
	
	Graph::DateType eDateType;
	
	switch (nDatePeriodSegment)
	{
		case 1:
			eDateType = Graph::DateWeek;
			break;
		case 2:
			eDateType = Graph::DateMonth;
			break;
		case 3:
			eDateType = Graph::DateYear;
			break;
		default:
			eDateType = Graph::DateCustom;
			break;
	}
	
	int nGraphViewType = [graphViewType indexOfTabViewItem:[graphViewType selectedTabViewItem]];
	Graph::ViewType eViewType = static_cast<Graph::ViewType>(nGraphViewType);
	
	int nItemsType = [graphItemTypes indexOfSelectedItem];
	Graph::ItemsType eItemsType = static_cast<Graph::ItemsType>(nItemsType);
	
	m_pGraph->setAccount(nAccount);
	m_pGraph->setViewType(eViewType);
	m_pGraph->setStartDate(startDate);
	m_pGraph->setEndDate(endDate);
	m_pGraph->setType(eType);
	m_pGraph->setIgnoreTransfers(bIgnoreTransfers);
	m_pGraph->setDateType(eDateType);
	m_pGraph->setItemsType(eItemsType);
	
	std::set<std::string> &aItems = m_pGraph->getItems();
	
	aItems.clear();
	
	for (NSString *sItem in m_aGraphItems)
	{
		std::string strItem = [sItem cStringUsingEncoding:NSUTF8StringEncoding];
		
		aItems.insert(strItem);
	}
	
	[self setDocumentModified:TRUE];
	
	[self redrawGraph:self];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	id result = @"";
	
	if (aTableView == graphItemsTableView)
	{
		NSString *sItem = [m_aGraphItems objectAtIndex:rowIndex];
		
		result = sItem;
	}
	
	return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	if (aTableView == graphItemsTableView)
		return [m_aGraphItems count];
	
	return 0;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{	
	if (rowIndex < 0)
		return;
	
	if (aTableView == graphItemsTableView)
	{
		NSString* sItem = object;
		
		[m_aGraphItems replaceObjectAtIndex:rowIndex withObject:sItem];
		
		[self redrawGraph:self];
	}
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	BOOL result = NO;
	
	if (aTableView == graphItemsTableView)
	{
		result = YES;
	}
	
	return result;
}

NSDate *convertToNSDate(MonthYear &date)
{
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];

	NSDateComponents *dateComponents = [[NSDateComponents alloc] init];
	[dateComponents setYear:date.getYear()];
	[dateComponents setMonth:date.getMonth()];
	[dateComponents setDay:1];

	[dateComponents setHour:0];
	[dateComponents setMinute:0];
	[dateComponents setSecond:0];

	NSDate *nsDate = [gregorian dateFromComponents:dateComponents];
	[dateComponents release];
	[gregorian release];

	return nsDate;
}

- (void)handleGraphSettingsUpdate:(NSNotification *)note
{
	[self redrawGraph:self];
}

- (void)setDocumentModified:(BOOL)modified
{
	[fMainController setDocumentModified:modified];
}

@end
