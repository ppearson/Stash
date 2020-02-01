/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009-2010 Peter Pearson
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

#include "storage.h"
#include "ofx.h"
#include "analysis.h"

#import "StashAppDelegate.h"
#import "IndexItem.h"
#import "AccountInfoController.h"
#import "SplitViewEx.h"
#import "ToolbarItemEx.h"
#import "TransactionsController.h"
#import "GraphController.h"
#import "ValueFormatter.h"

#define TOOLBAR_ADDACCOUNT		@"TOOLBAR_ADDACCOUNT"
#define TOOLBAR_ADDGRAPH		@"TOOLBAR_ADDGRAPH"
#define TOOLBAR_VIEWRECENT		@"TOOLBAR_VIEWRECENT"
#define TOOLBAR_VIEWTHISYEAR	@"TOOLBAR_VIEWTHISYEAR"
#define TOOLBAR_VIEWALL			@"TOOLBAR_VIEWALL"
#define TOOLBAR_VIEWTYPE		@"TOOLBAR_VIEWTYPE"
#define TOOLBAR_MAKETRANSFER	@"TOOLBAR_MAKETRANSFER"

typedef enum
{
    TOOLBAR_VIEW_RECENT_TAG = 0,
    TOOLBAR_VIEW_THISYEAR_TAG = 1,
	TOOLBAR_VIEW_ALL_TAG = 2
}
toolbarViewGroupTag;

@implementation StashAppDelegate

@synthesize window;
@synthesize ShowTransactionsViewType;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{	
	m_HasFinishedLoading = true;
	
	if (m_sPendingOpenFile != nil)
	{
		[self application:nil openFile:m_sPendingOpenFile];
	}
	else
	{
		if ([[NSUserDefaults standardUserDefaults] boolForKey:@"GeneralOpenLastFile"] == YES)
		{
			NSArray *aDocuments = [[NSDocumentController sharedDocumentController] recentDocumentURLs];
			
			if ([aDocuments count] > 0)
			{
				NSURL *url = [aDocuments objectAtIndex:0];
				
				[self application:nil openFile:[url path]];
				
				return;
			}
		}
		
		[self buildIndexTree];
		
		[self calculateAndShowScheduled];
	
		[window makeKeyAndOrderFront:self];
	}
}

- (id)init
{
	self = [super init];
	if (self)
	{
		prefController = [[PreferencesController alloc] init];
		
		NSNotificationCenter *nc;
		nc = [NSNotificationCenter defaultCenter];
		[nc addObserver:self selector:@selector(handleGraphSettingsUpdate:) name:@"GraphSettingsUpdate" object:nil];
		
		m_HasFinishedLoading = false;
		m_sPendingOpenFile = nil;
		
		[NSApp setDelegate: (id)self];
	}
	
	return self;
}

- (void)dealloc
{
	[prefController release];
	
	NSNotificationCenter *nc;
	nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self];
	
	[super dealloc];
}

+ (void)initialize
{
	NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
	
	[defaultValues setObject:[NSNumber numberWithBool:NO] forKey:@"GeneralOpenLastFile"];
	[defaultValues setObject:[NSNumber numberWithBool:NO] forKey:@"GeneralCreateBackupOnSave"];
	
	[defaultValues setObject:[NSNumber numberWithInt:30] forKey:@"TransactionsRecentDuration"];
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"TransactionsScrollToLatest"];
	[defaultValues setObject:[NSNumber numberWithBool:NO] forKey:@"TransactionsNegAmountsRed"];
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"TransactionsNegBalancesRed"];
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"TransactionsEnforceNegForCategories"];
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"TransactionsEnforceNegForSplit"];
    [defaultValues setObject:[NSNumber numberWithBool:NO] forKey:@"TransactionsAreClearedByDefault"];
	
	[defaultValues setObject:[NSNumber numberWithInt:0] forKey:@"PieChartSortType"];
	[defaultValues setObject:[NSNumber numberWithInt:0] forKey:@"PieChartSelectionType"];
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"PieChartGroupSmallerItems"];
	[defaultValues setObject:[NSNumber numberWithInt:3] forKey:@"PieChartGroupSmallerItemsSize"];
	[defaultValues setValue:@"Other" forKey:@"PieChartGroupSmallerItemsName"];
	
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"AreaChartGroupSmallerItems"];
	[defaultValues setObject:[NSNumber numberWithInt:2] forKey:@"AreaChartGroupSmallerItemsSize"];
	[defaultValues setValue:@"Other" forKey:@"AreaChartGroupSmallerItemsName"];
	
	NSDictionary *visibleTransactionColumns = [NSDictionary dictionaryWithObjectsAndKeys:
									   [NSNumber numberWithBool:YES], @"Cleared",
									   [NSNumber numberWithBool:YES], @"Date",
									   [NSNumber numberWithBool:YES], @"Payee",
									   [NSNumber numberWithBool:YES], @"Category",
									   [NSNumber numberWithBool:YES], @"Description",
									   [NSNumber numberWithBool:NO], @"Type",
									   [NSNumber numberWithBool:YES], @"Amount",
									   [NSNumber numberWithBool:YES], @"Balance",
									   nil];
	
	NSDictionary *transactionColumnSizes = [NSDictionary dictionaryWithObjectsAndKeys:
									 [NSNumber numberWithFloat:38], @"Cleared",
									 [NSNumber numberWithFloat:86], @"Date",
									 [NSNumber numberWithFloat:147], @"Payee",
									 [NSNumber numberWithFloat:147], @"Category",
									 [NSNumber numberWithFloat:157], @"Description",
									 [NSNumber numberWithFloat:80], @"Type",
									 [NSNumber numberWithFloat:91], @"Amount",
									 [NSNumber numberWithFloat:83], @"Balance",
									 nil];
	
	NSArray *transactionColumnOrder = [NSArray arrayWithObjects:@"Cleared", @"Date", @"Payee", @"Category", @"Description", @"Amount", @"Balance", nil];
	
	[defaultValues setObject:visibleTransactionColumns forKey:@"VisibleTransactionColumns"];
	[defaultValues setObject:transactionColumnSizes forKey:@"TransactionColumnSizes"];
	[defaultValues setObject:transactionColumnOrder forKey:@"TransactionColumnOrder"];
		
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
}

- (void)awakeFromNib
{
	[window setDelegate:(id)self];
	
	[indexView setFrameSize:[indexViewPlaceholder frame].size];
	[indexViewPlaceholder addSubview:indexView];
	
	[vTransactionsView setFrameSize:[contentViewPlaceholder frame].size];
	
	[contentViewPlaceholder addSubview:vTransactionsView];
	contentView = vTransactionsView;
	
	nViewType = 0;
	
	NSToolbar * toolbar = [[NSToolbar alloc] initWithIdentifier:@"Toolbar"];
    [toolbar setDelegate:(id)self];
    [toolbar setAllowsUserCustomization:NO];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    [toolbar setSizeMode:NSToolbarSizeModeRegular];
    [[self window] setToolbar:toolbar];
    [toolbar release];
	
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC setWindow:[self window]];
	[pTC setIndexBar:indexBar];
	[pTC setMainController:self];
	
	GraphController* pGC = [GraphController sharedInterface];
	[pGC setMainController:self];
	
	// Load the splitter positions
	[indexBarSplitView loadLayoutFromDefault:@"indexSplitter"];

	[scheduledverticalSplitView loadLayoutFromDefault:@"schedtransSplitter"];
	
	NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
	
	float fPosX = [[defs objectForKey:@"MainWndPosX"] floatValue];
	float fPosY = [[defs objectForKey:@"MainWndPosY"] floatValue];
	float fWidth = [[defs objectForKey:@"MainWndWidth"] floatValue];
	float fHeight = [[defs objectForKey:@"MainWndHeight"] floatValue];
	
	if (fPosX >= 0.0 && fPosY >= 0.0 && fWidth > 0.0 && fHeight > 0.0)
	{
		NSScreen *mainScreen = [NSScreen mainScreen];
		NSRect screenRect = [mainScreen frame];
		
		if (fWidth <= screenRect.size.width && fHeight <= screenRect.size.height)
		{
			NSRect windowRect = NSMakeRect(fPosX, fPosY, fWidth, fHeight);
			
			[window setFrame:windowRect display:YES animate:NO];
		}
	}
	
	m_aPayeeItems = [[NSMutableArray alloc] init];
	m_aCategoryItems = [[NSMutableArray alloc] init];
	m_aScheduledTransactions = [[NSMutableArray alloc] init];
	
	NSDate *date1 = [NSDate date];
	[scheduledDateCntl setDateValue:date1];
		
	[scheduledType removeAllItems];
	
	[scheduledType addItemWithTitle:NSLocalizedString(@"None", "Transaction Type -> None")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Deposit", "Transaction Type -> Deposit")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Withdrawal", "Transaction Type -> Withdrawal")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Transfer", "Transaction Type -> Transfer")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Standing Order", "Transaction Type -> Standing Order")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Direct Debit", "Transaction Type -> Direct Debit")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Point Of Sale", "Transaction Type -> Point Of Sale")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Charge", "Transaction Type -> Charge")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"ATM", "Transaction Type -> ATM")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Check", "Transaction Type -> Check")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Credit", "Transaction Type -> Credit")];
	[scheduledType addItemWithTitle:NSLocalizedString(@"Debit", "Transaction Type -> Debit")];
	
	[scheduledFrequency removeAllItems];
	
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Weekly", "Scheduled Transactions Pane -> Frequency -> Weekly")];
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Two Weeks", "Scheduled Transactions Pane -> Frequency -> Two Weeks")];
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Four Weeks", "Scheduled Transactions Pane -> Frequency -> Four Weeks")];
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Monthly", "Scheduled Transactions Pane -> Frequency -> Monthly")];
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Two Months", "Scheduled Transactions Pane -> Frequency -> Two Months")];
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Quarterly", "Scheduled Transactions Pane -> Frequency -> Quarterly")];
	[scheduledFrequency addItemWithTitle:NSLocalizedString(@"Annually", "Scheduled Transactions Pane -> Frequency -> Annually")];
	
	[scheduledConstraint removeAllItems];
	
	[scheduledConstraint addItemWithTitle:NSLocalizedString(@"Exact Day", "Scheduled Transactions Pane -> Constraint -> Exact Day")];
	[scheduledConstraint addItemWithTitle:NSLocalizedString(@"Exact or Next Working Day", "Scheduled Transactions Pane -> Constraint -> Exact or Next Working Day")];
	[scheduledConstraint addItemWithTitle:NSLocalizedString(@"Last Working Day of Month", "Scheduled Transactions Pane -> Constraint -> Last Working Day of Month")];
	
	[deleteScheduled setEnabled:NO];
	
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(ScheduledSelectionDidChange:) name:NSTableViewSelectionDidChangeNotification object:scheduledTransactionsTableView];
	
	[payeesTableView setDelegate:(id)self];
	[categoriesTableView setDelegate:(id)self];
	[scheduledTransactionsTableView setDelegate:(id)self];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)ident willBeInsertedIntoToolbar:(BOOL)flag
{	
    if ([ident isEqualToString:TOOLBAR_ADDACCOUNT])
    {
		NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:ident];
		
        [item setLabel:NSLocalizedString(@"Add Account", "Main Toolbar -> Add Account")];
        [item setImage:[NSImage imageNamed:@"add_account.png"]];
        [item setTarget:self];
        [item setAction:@selector(AddAccount:)];
        [item setAutovalidates:NO];
		
		return item;
    }
    else if ([ident isEqualToString:TOOLBAR_ADDGRAPH])
    {
		NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:ident];
		
        [item setLabel:NSLocalizedString(@"Add Graph", "Main Toolbar -> Add Graph")];
        [item setImage:[NSImage imageNamed:@"add_graph.png"]];
        [item setTarget:self];
        [item setAction:@selector(AddGraph:)];
        [item setAutovalidates:NO];
		
		return item;
    }
	else if ([ident isEqualToString:TOOLBAR_MAKETRANSFER])
    {
		NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:ident];
		
        [item setLabel:NSLocalizedString(@"Make Transfer", "Main Toolbar -> Make Transfer")];
        [item setImage:[NSImage imageNamed:@"make_transfer.png"]];
        [item setTarget:self];
        [item setAction:@selector(MakeTransfer:)];
        [item setAutovalidates:YES];
		
		return item;
    }
	else if ([ident isEqualToString:TOOLBAR_VIEWTYPE])
    {
        ToolbarItemEx *groupItem = [[ToolbarItemEx alloc] initWithItemIdentifier:ident];
        
        NSSegmentedControl *segmentedControl = [[NSSegmentedControl alloc] initWithFrame:NSZeroRect];
        [segmentedControl setCell:[[[NSSegmentedCell alloc] init] autorelease]];
        [groupItem setView:segmentedControl];
        NSSegmentedCell *segmentedCell = (NSSegmentedCell *)[segmentedControl cell];
        
        [segmentedControl setSegmentCount:3];
        [segmentedCell setTrackingMode:NSSegmentSwitchTrackingSelectOne];
		
		[segmentedControl setSegmentStyle:NSSegmentStyleTexturedSquare];
        
        const NSSize groupSize = NSMakeSize(180.0, 25.0);
        [groupItem setMinSize:groupSize];
        [groupItem setMaxSize:groupSize];
        
        [groupItem setLabel:NSLocalizedString(@"View Type", "Main Toolbar -> View Type")];
        [groupItem setPaletteLabel:NSLocalizedString(@"View Type", "Main Toolbar -> View Type")];
        [groupItem setTarget:self];
        [groupItem setAction:@selector(viewToolbarClicked:)];
        
        [segmentedCell setTag:TOOLBAR_VIEW_RECENT_TAG forSegment:TOOLBAR_VIEW_RECENT_TAG];
		[segmentedControl setLabel:NSLocalizedString(@"Recent", "Main Toolbar -> View Type -> Recent") forSegment:TOOLBAR_VIEW_RECENT_TAG];
        [segmentedCell setToolTip:NSLocalizedString(@"Show recent Transactions", "Main Toolbar -> View Type -> Recent Tooltip") forSegment:TOOLBAR_VIEW_RECENT_TAG];
        
        [segmentedCell setTag:TOOLBAR_VIEW_THISYEAR_TAG forSegment:TOOLBAR_VIEW_THISYEAR_TAG];
		[segmentedControl setLabel:NSLocalizedString(@"This Year", "Main Toolbar -> View Type -> This Year") forSegment:TOOLBAR_VIEW_THISYEAR_TAG];
        [segmentedCell setToolTip:NSLocalizedString(@"Show Transactions from this year", "Main Toolbar -> View Type -> This Year Tooltip") forSegment:TOOLBAR_VIEW_THISYEAR_TAG];
		
		[segmentedCell setTag: TOOLBAR_VIEW_ALL_TAG forSegment:TOOLBAR_VIEW_ALL_TAG];
		[segmentedControl setLabel:NSLocalizedString(@"All", "Main Toolbar -> View Type -> All") forSegment:TOOLBAR_VIEW_ALL_TAG];
        [segmentedCell setToolTip:NSLocalizedString(@"Show all Transactions", "Main Toolbar -> View Type -> All Tooltip") forSegment:TOOLBAR_VIEW_ALL_TAG];
		
		TransactionsController* pTC = [TransactionsController sharedInterface];
		[segmentedControl bind:@"selectedIndex" toObject:pTC withKeyPath:@"m_showTransactionsViewType" options:0];
		
		NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
		
		int nTransactionsViewType = [[defs objectForKey:@"TransactionsViewType"] intValue];
		
		[segmentedControl setSelectedSegment:nTransactionsViewType];
        
        [segmentedControl release];
        return [groupItem autorelease];
    }
	
    return nil;
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    return [self toolbarAllowedItemIdentifiers:toolbar];
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    return [NSArray arrayWithObjects:TOOLBAR_ADDACCOUNT, TOOLBAR_ADDGRAPH, TOOLBAR_VIEWTYPE, TOOLBAR_MAKETRANSFER, nil];
}

- (void)buildIndexTree
{
	[indexBar clearAllItems];
	
	[indexBar addSection:@"accounts" title:NSLocalizedString(@"ACCOUNTS", "IndexBar -> ACCOUNTS")];
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	std::vector<Account>::iterator it = m_Document.AccountBegin();
	int nAccount = 0;
	
	for (; it != m_Document.AccountEnd(); ++it, nAccount++)
	{
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		
		fixed accBalance = it->getBalance(true);
		
		NSString *sAccBalance = [[valueFormatter currencyStringFromFixed:accBalance] retain];
		
		NSString *sAccountKey = [NSString stringWithFormat:@"a%d", nAccount];

		[indexBar addItem:@"accounts" key:sAccountKey title:sName amount:sAccBalance item:nAccount action:@selector(accountSelected:) target:self type:1 rename:@selector(accountRenamed:) renameTarget:self];
		
		[sName release];
	}
	
	[indexBar addSection:@"manage" title:NSLocalizedString(@"MANAGE", "IndexBar -> MANAGE")];
	[indexBar addItem:@"manage" key:@"payees" title:NSLocalizedString(@"Payees", "IndexBar -> Payees") item:0 action:@selector(payeesSelected:) target:self type:2 rename:nil renameTarget:nil];
	[indexBar addItem:@"manage" key:@"categories" title:NSLocalizedString(@"Categories", "IndexBar -> Categories") item:0 action:@selector(categoriesSelected:) target:self type:2 rename:nil renameTarget:nil];
	[indexBar addItem:@"manage" key:@"scheduled" title:NSLocalizedString(@"Scheduled", "IndexBar -> Scheduled") item:0 action:@selector(scheduledSelected:) target:self type:2 rename:nil renameTarget:nil];
	
	[indexBar addSection:@"graphs" title:NSLocalizedString(@"GRAPHS", "IndexBar -> GRAPHS")];
	
	std::vector<Graph>::iterator itGraph = m_Document.GraphBegin();
	int nGraph = 0;
	
	for (; itGraph != m_Document.GraphEnd(); ++itGraph, nGraph++)
	{
		std::string strName = itGraph->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		
		NSString *sGraphKey = [NSString stringWithFormat:@"g%d", nGraph];
		
		[indexBar addItem:@"graphs" key:sGraphKey title:sName item:nGraph action:@selector(graphSelected:) target:self type:3 rename:@selector(graphRenamed:) renameTarget:self];
		
		[sName release];
	}
	
	m_pAccount = 0;
	
	[indexBar reloadData];
	
	[indexBar expandSection:@"accounts"];
	[indexBar expandSection:@"manage"];
	[indexBar expandSection:@"graphs"];
	
	if (nAccount > 0)
	{
		// automatically select the first account
		[indexBar selectItem:@"a0"];
	}
}

- (void)accountSelected:(id)sender
{
	m_bEditing = false;
	
	int nAccount = [sender getItemIndex];
		
	m_pAccount = &m_Document.getAccount(nAccount);
	
	[vTransactionsView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vTransactionsView];
	contentView = vTransactionsView;
	
	nViewType = 0;
	
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC showTransactionsForAccount:m_pAccount];
	
	[self refreshLibraryItems];
}

- (void)payeesSelected:(id)sender
{
	m_bEditing = false;
	
	[vPayeesView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vPayeesView];
	contentView = vPayeesView;
	
	nViewType = 1;
	
	[self buildPayeesList];
}

- (void)categoriesSelected:(id)sender
{
	m_bEditing = false;
	
	[vCategoriesView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vCategoriesView];
	contentView = vCategoriesView;
	
	nViewType = 2;
	
	[self buildCategoriesList];
}

- (void)scheduledSelected:(id)sender
{
	m_bEditing = false;
	
	m_pAccount = 0;
	
	[vScheduledView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vScheduledView];
	contentView = vScheduledView;
	
	nViewType = 3;
	
	// Update the list of Accounts
	
	[scheduledAccount removeAllItems];
	
	std::vector<Account>::iterator it = m_Document.AccountBegin();
	
	for (; it != m_Document.AccountEnd(); ++it)
	{
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		[scheduledAccount addItemWithTitle:sName];
		
		[sName release];
	}
	
	[scheduledTransactionsTableView deselectAll:self];
	
	[self refreshLibraryItems];
	
	[self buildSchedTransList];
}

- (void)graphSelected:(id)sender
{
	m_bEditing = false;
	
	m_pAccount = 0;
	
	int nGraph = [sender getItemIndex];
	
	Graph* pGraph = &m_Document.getGraph(nGraph);
	
	if (!pGraph)
		return;
	
	[vGraphView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vGraphView];
	contentView = vGraphView;
	
	nViewType = 4;
	
	GraphController* pGC = [GraphController sharedInterface];
	[pGC showGraph:pGraph];
}

// handle renames in the IndexBar

- (void)accountRenamed:(id)sender
{
	int nAccount = [sender getItemIndex];
	
	m_pAccount = &m_Document.getAccount(nAccount);
	
	NSString *title = [sender title];
	
	std::string strName = [title cStringUsingEncoding:NSUTF8StringEncoding];
	
	m_pAccount->setName(strName);
	
	[self setDocumentModified:TRUE];
	
	[indexBar reloadData];
	[indexBar setNeedsDisplay:YES];
}

- (void)graphRenamed:(id)sender
{
	int nGraph = [sender getItemIndex];
	
	Graph* pGraph = &m_Document.getGraph(nGraph);
	
	if (!pGraph)
		return;
	
	NSString *title = [sender title];
	
	std::string strName = [title cStringUsingEncoding:NSUTF8StringEncoding];
	
	pGraph->setName(strName);
	
	[self setDocumentModified:TRUE];
	
	[indexBar reloadData];
}

- (void)setWindowTitleWithDocName:(NSString*)path
{
	NSString *sWindowTitle;
	if (path)
	{
		sWindowTitle = [NSString stringWithFormat:@"Stash - %@", [path lastPathComponent]];
	}
	else
	{
		sWindowTitle = @"Stash";
	}
	
	[[self window] setTitle:sWindowTitle];	
}

- (void)buildPayeesList
{
	[m_aPayeeItems removeAllObjects];
		
	std::set<std::string>::iterator it = m_Document.PayeeBegin();
	
	for (; it != m_Document.PayeeEnd(); ++it)
	{
		std::string strPayee = (*it);
		NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
			
		[m_aPayeeItems addObject:sPayee];
		[sPayee release];
	}
	
	[payeesTableView reloadData];	
}

- (void)buildCategoriesList
{
	[m_aCategoryItems removeAllObjects];
	
	std::set<std::string>::iterator it = m_Document.CategoryBegin();
	
	for (; it != m_Document.CategoryEnd(); ++it)
	{
		std::string strCategory = (*it);
		NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
		
		[m_aCategoryItems addObject:sCategory];
		[sCategory release];
	}
	
	[categoriesTableView reloadData];	
}

- (void)buildSchedTransList
{
	[m_aScheduledTransactions removeAllObjects];
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	std::vector<ScheduledTransaction>::iterator it = m_Document.SchedTransBegin();
	
	int schedTransIndex = 0;
	
	for (; it != m_Document.SchedTransEnd(); ++it, schedTransIndex++)
	{
		NSMutableDictionary *item = [[NSMutableDictionary alloc] init];
		
		std::string strSPayee = it->getPayee();
		NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
		
		std::string strSCategory = it->getCategory();
		NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
		
		NSString *sSAmount = [[valueFormatter currencyStringFromFixed:it->getAmount()] retain];
		
		NSDate *date = convertToNSDate(it->getNextDate());
		NSString *sSDate = [[dateFormatter stringFromDate:date] retain];
		
		int nFreq = it->getFrequency();
		NSString *sFrequency = [scheduledFrequency itemTitleAtIndex:nFreq];
		
		int nAccount = it->getAccount();
		int nNumAccounts = [scheduledAccount numberOfItems];
		
		NSString *sAccount = @"";
		if (nAccount >= 0 && nAccount < nNumAccounts)
		{
			sAccount = [scheduledAccount itemTitleAtIndex:nAccount];
		}
		
		bool isEnabled = it->isEnabled();
		
		[item setValue:[NSNumber numberWithBool:isEnabled] forKey:@"enabled"];
		[item setValue:[NSNumber numberWithInt:schedTransIndex] forKey:@"index"];
		[item setValue:sSPayee forKey:@"payee"];
		[item setValue:sSCategory forKey:@"category"];
		[item setValue:sSAmount forKey:@"amount"];
		[item setValue:sSDate forKey:@"nextdate"];
		[item setValue:sFrequency forKey:@"frequency"];
		[item setValue:sAccount forKey:@"account"];
        
        [sSCategory release];
        [sSAmount release];
        [sSDate release];
		
		[m_aScheduledTransactions addObject:item];
        
        [item release];
	}
	
	[dateFormatter release];
	
	[scheduledTransactionsTableView reloadData];
}

- (void)refreshLibraryItems
{
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC refreshLibraryItems];
	
	[scheduledPayee removeAllItems];
	
	std::set<std::string>::iterator it = m_Document.PayeeBegin();
	
	for (; it != m_Document.PayeeEnd(); ++it)
	{
		NSString *sPayee = [[NSString alloc] initWithUTF8String:(*it).c_str()];
		
		[scheduledPayee addItemWithObjectValue:sPayee];
		
		[sPayee release];
	}
	
	[scheduledCategory removeAllItems];
	
	std::set<std::string>::iterator itCat = m_Document.CategoryBegin();
	
	for (; itCat != m_Document.CategoryEnd(); ++itCat)
	{
		NSString *sCategory = [[NSString alloc] initWithUTF8String:(*itCat).c_str()];
		
		[scheduledCategory addItemWithObjectValue:sCategory];
		
		[sCategory release];
	}
}

- (IBAction)AddAccount:(id)sender
{
	AddAccountController *addAccountController = [[AddAccountController alloc] initWnd:self];
	[addAccountController showWindow:self];
}

- (void)addAccountConfirmed:(AddAccountController *)addAccountController
{
	NSString *sAccountName = [addAccountController accountName];
	NSString *sStartingBalance = [addAccountController startingBalance];
	NSString *sInstitution = [addAccountController institution];
	NSString *sNumber = [addAccountController number];
	NSString *sNote = [addAccountController note];
	
	AccountType eType = [addAccountController accountType];
	
	std::string strName = [sAccountName cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strInstitution = [sInstitution cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strNumber = [sNumber cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strNote = [sNote cStringUsingEncoding:NSUTF8StringEncoding];
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	fixed startingBalance = [valueFormatter fixedFromString:sStartingBalance];
	
	Account newAccount;
	newAccount.setName(strName);
	newAccount.setType(eType);
	newAccount.setInstitution(strInstitution);
	newAccount.setNumber(strNumber);
	newAccount.setNote(strNote);
	
	// only add a starting balance if a value is given
	
	if (!startingBalance.IsZero())
	{
		Date currentDate;
		currentDate.Now();
		
		NSString *sStartingBalanceText = NSLocalizedString(@"Starting balance", "New Account -> Starting balance");
		std::string strStartingBalanceText = [sStartingBalanceText cStringUsingEncoding:NSUTF8StringEncoding];
		Transaction newTransaction(strStartingBalanceText, "", "", startingBalance, currentDate);
		newTransaction.setCleared(true);
		
		newAccount.addTransaction(newTransaction);
	}
	
	// format starting balance to string again for IndexBar
	
	NSString *sBalance = [[valueFormatter currencyStringFromFixed:startingBalance] retain];
	
	int nAccountNum = m_Document.addAccount(newAccount);
	
	NSString *sAccountKey = [NSString stringWithFormat:@"a%d", nAccountNum];
	
	[indexBar addItem:@"accounts" key:sAccountKey title:sAccountName amount:sBalance item:nAccountNum action:@selector(accountSelected:) target:self type:1 rename:@selector(accountRenamed:) renameTarget:self];
	
	[addAccountController release];
	
	[self setDocumentModified:TRUE];
	
	[indexBar reloadData];
	
	if (nAccountNum == 0) // if first account added, select it
	{
		[indexBar selectItem:sAccountKey];
        
        // update data structures
        m_pAccount = &m_Document.getAccount(0);
		TransactionsController* pTC = [TransactionsController sharedInterface];
		[pTC showTransactionsForAccount:m_pAccount];
	}
	else // otherwise, we need to update m_pAccount as that will have become invalidated
	{
		int nSelectedAccount = [indexBar getItemIndex];
		
		m_pAccount = &m_Document.getAccount(nSelectedAccount);
		TransactionsController* pTC = [TransactionsController sharedInterface];
		[pTC showTransactionsForAccount:m_pAccount];
	}
}

- (IBAction)AccountInfo:(id)sender
{
	int nAccount = [indexBar getItemIndex];
	
	if (nAccount >= 0)
	{
		Account &oAccount = m_Document.getAccount(nAccount);
		
		NSString *sName = [[NSString alloc] initWithUTF8String:oAccount.getName().c_str()];
		NSString *sInstitution = [[NSString alloc] initWithUTF8String:oAccount.getInstitution().c_str()];
		NSString *sNumber = [[NSString alloc] initWithUTF8String:oAccount.getNumber().c_str()];
		NSString *sNote = [[NSString alloc] initWithUTF8String:oAccount.getNote().c_str()];
		AccountType eType = oAccount.getType();		
		
		AccountInfoController *accountInfoController = [[AccountInfoController alloc] initWnd:self withAccount:nAccount name:sName institution:sInstitution number:sNumber
																					 note:sNote type:eType];
		[accountInfoController showWindow:self];
		
		[sName release];
		[sInstitution release];
		[sNumber release];
		[sNote release];
	}	
}

- (void)updateAccountInfo:(int)account name:(NSString*)name institution:(NSString*)institution
				   number:(NSString*)number note:(NSString*)note type:(AccountType)type
{
	Account &oAccount = m_Document.getAccount(account);
	
	std::string strName = [name cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strInstitution = [institution cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strNumber = [number cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strNote = [note cStringUsingEncoding:NSUTF8StringEncoding];
		
	oAccount.setName(strName);
	oAccount.setType(type);
	oAccount.setInstitution(strInstitution);
	oAccount.setNumber(strNumber);
	oAccount.setNote(strNote);
	
	[self setDocumentModified:TRUE];
	
	[self buildIndexTree];
}

- (IBAction)DeleteAccount:(id)sender
{
	m_bEditing = false;
	
	int nAccount = [indexBar getItemIndex];
	
	if (nAccount >= 0)
	{
		NSString * message = NSLocalizedString(@"The Account and all transactions in it will be deleted. Are you sure?", "Account Deletion Warning Message");
		
		int choice = NSAlertDefaultReturn;
		
		choice = NSRunAlertPanel(NSLocalizedString(@"Delete Account?", "Account Deletion Warning Title"), message, NSLocalizedString(@"Delete", "Delete Button"), NSLocalizedString(@"Cancel", "Cancel Button"), nil);
		
		if (choice != NSAlertDefaultReturn)
			return;
		
		m_Document.deleteAccount(nAccount);
		
		m_Document.disabledScheduledTransactionsForAccount(nAccount);
		
		[self setDocumentModified:TRUE];
		
		[self buildIndexTree];
	}
}

- (IBAction)MakeTransfer:(id)sender
{
	if (m_Document.getAccountCount() < 2)
	{
		NSRunAlertPanel(NSLocalizedString(@"Not enough accounts", "Not enough accounts Warning Title"),
						NSLocalizedString(@"You must have more than one account in the current document to create a Transfer.", "Not enough accounts Warning Message"),
						NSLocalizedString(@"OK", "OK Button"), nil, nil);
		return;
	}
	
	NSMutableArray *aAccounts = [[NSMutableArray alloc] init];
	
	std::vector<Account>::iterator it = m_Document.AccountBegin();
	
	for (; it != m_Document.AccountEnd(); ++it)
	{
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		[aAccounts addObject:sName];
		
		[sName release];
	}
	
	NSMutableArray *aCategories = [[NSMutableArray alloc] init];
	
	std::set<std::string>::iterator itCat = m_Document.CategoryBegin();
	
	for (; itCat != m_Document.CategoryEnd(); ++itCat)
	{
		NSString *sCategory = [[NSString alloc] initWithUTF8String:(*itCat).c_str()];
		
		[aCategories addObject:sCategory];
	}
	
	if (makeTransferController)
	{
		[makeTransferController release];
	}
	
	makeTransferController = [[MakeTransferController alloc] initWithAccounts:aAccounts categories:aCategories];
	
	[makeTransferController makeTransfer:window initialAccount:0];	
}

- (void)makeTransferItem:(MakeTransferController *)makeTransferCont
{
	// pass it through to the TransactionsController
	
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC makeTransferItem:makeTransferCont];
}

- (void)ScheduledSelectionDidChange:(NSNotification *)notification
{	
	NSIndexSet *rows = [scheduledTransactionsTableView selectedRowIndexes];
	
	if ([rows count] == 1)
	{
		[deleteScheduled setEnabled:YES];
		
		m_bEditing = true;
		
		NSInteger row = [rows lastIndex];

		NSMutableDictionary *item = [[m_aScheduledTransactions objectAtIndex:row] retain];
		
		ScheduledTransaction &schedTrans = m_Document.getScheduledTransaction(row);
		
		Transaction::Type eType = schedTrans.getType();
		
		Date date1 = schedTrans.getNextDate();
		NSDate *datetemp = convertToNSDate(date1);
		
		int account = schedTrans.getAccount();
		
		[scheduledPayee setStringValue:[item valueForKey:@"payee"]];
		[scheduledCategory setStringValue:[item valueForKey:@"category"]];
		
		std::string strDescription = schedTrans.getDescription();
		NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
		[scheduledDescription setStringValue:sDescription];
		
		[scheduledAmount setStringValue:[item valueForKey:@"amount"]];
		
		ScheduledTransaction::Frequency eFreq = schedTrans.getFrequency();
		
		[scheduledFrequency selectItemAtIndex:eFreq];
		[scheduledType selectItemAtIndex:eType];
		
		ScheduledTransaction::Constraint eConstraint = schedTrans.getConstraint();
		[scheduledConstraint selectItemAtIndex:eConstraint];
		
		[scheduledDateCntl setDateValue:datetemp];
		
		if (account >= 0)
		{
			[scheduledAccount selectItemAtIndex:account];
		}
	}
	else
	{
		m_bEditing = false;
		
		[deleteScheduled setEnabled:NO];
		
		[scheduledPayee setStringValue:@""];
		[scheduledCategory setStringValue:@""];
		[scheduledDescription setStringValue:@""];
		[scheduledAmount setStringValue:@""];
	}		
}

- (void)updateScheduled:(id)sender
{
	if (!m_bEditing)
		return;
	
	NSIndexSet *rows = [scheduledTransactionsTableView selectedRowIndexes];
	
	if ([rows count] != 1)
	{		
		return;
	}
	
	NSInteger row = [rows lastIndex];
	
	ScheduledTransaction &schedTrans = m_Document.getScheduledTransaction(row);
	
	NSDate *ndate1 = [scheduledDateCntl dateValue];
	NSCalendarDate *CalDate = [ndate1 dateWithCalendarFormat:0 timeZone:0];
	
	int nYear = [CalDate yearOfCommonEra];
	int nMonth = [CalDate monthOfYear];
	int nDay = [CalDate dayOfMonth];
	
	Date date1(nDay, nMonth, nYear);
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	NSString *sDate = [[dateFormatter stringFromDate:ndate1] retain];
	
	[dateFormatter release];
	
	std::string strPayee = [[scheduledPayee stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strDesc = [[scheduledDescription stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strCategory = [[scheduledCategory stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
	
	NSString *sScheduledAmount = [scheduledAmount stringValue];

	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	fixed fAmount = [valueFormatter fixedFromString:sScheduledAmount];
	
	// reformat the number again, in case an abbreviation was used
	NSString *sAmount = [[valueFormatter currencyStringFromFixed:fAmount] retain];
	
	int nType = [scheduledType indexOfSelectedItem];
	Transaction::Type eType = static_cast<Transaction::Type>(nType);
	
	int nAccount = [scheduledAccount indexOfSelectedItem];
	
	int nFrequency = [scheduledFrequency indexOfSelectedItem];
	ScheduledTransaction::Frequency eFreq = static_cast<ScheduledTransaction::Frequency>(nFrequency);
	
	int nConstraint = [scheduledConstraint indexOfSelectedItem];
	ScheduledTransaction::Constraint eConstraint = static_cast<ScheduledTransaction::Constraint>(nConstraint);
	
	// update the actual ScheduledTransaction
	
	schedTrans.setAccount(nAccount);
	schedTrans.setPayee(strPayee);
	schedTrans.setCategory(strCategory);
	schedTrans.setDescription(strDesc);
	schedTrans.setAmount(fAmount);
	schedTrans.setFrequency(eFreq);
	schedTrans.setNextDate(date1);
	schedTrans.setType(eType);
	schedTrans.setConstraint(eConstraint);
	
	NSMutableDictionary *oSchedTransItem = [m_aScheduledTransactions objectAtIndex:row];
	
	[oSchedTransItem setValue:[scheduledPayee stringValue] forKey:@"payee"];
	[oSchedTransItem setValue:[scheduledDescription stringValue] forKey:@"description"];
	[oSchedTransItem setValue:[scheduledCategory stringValue] forKey:@"category"];
	[oSchedTransItem setValue:sAmount forKey:@"amount"];
	[oSchedTransItem setValue:[scheduledFrequency titleOfSelectedItem] forKey:@"frequency"];
	[oSchedTransItem setValue:sDate forKey:@"nextdate"];
	[oSchedTransItem setValue:[scheduledAccount titleOfSelectedItem] forKey:@"account"];	
	
	[scheduledTransactionsTableView reloadData];
	
	[self setDocumentModified:TRUE];
}

- (IBAction)deleteMisc:(id)sender
{
	m_bEditing = false;
	
	int nGraph = [indexBar getItemIndex];
	
	if (nGraph >= 0)
	{
		NSString * message = NSLocalizedString(@"This Graph will be deleted. Are you sure?", "Graph Deletion Warning Message");
		
		int choice = NSAlertDefaultReturn;
		
		choice = NSRunAlertPanel(NSLocalizedString(@"Delete Graph?", "Graph Deletion Warning Title"), message,
								 NSLocalizedString(@"Delete", "Delete Button"), NSLocalizedString(@"Cancel", "Cancel Button"), nil);
		
		if (choice != NSAlertDefaultReturn)
			return;
		
		m_Document.deleteGraph(nGraph);
		
		[self setDocumentModified:TRUE];
		
		[self buildIndexTree];
	}	
}

- (IBAction)showRecentTransactions:(id)sender
{
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC setTransactionsViewType:RECENT];
}

- (IBAction)showAllTransactionsThisYear:(id)sender
{	
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC setTransactionsViewType:ALL_THIS_YEAR];
}

- (IBAction)showAllTransactions:(id)sender
{
	TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC setTransactionsViewType:ALL];
}

- (void)viewToolbarClicked:(id)sender
{
	NSInteger tagValue = [sender isKindOfClass: [NSSegmentedControl class]]
	? [(NSSegmentedCell *)[sender cell] tagForSegment: [sender selectedSegment]] : [sender tag];
	
    switch (tagValue)
    {
        case TOOLBAR_VIEW_RECENT_TAG:
            [self showRecentTransactions: sender];
            break;
        case TOOLBAR_VIEW_THISYEAR_TAG:
            [self showAllTransactionsThisYear: sender];
            break;
		case TOOLBAR_VIEW_ALL_TAG:
            [self showAllTransactions: sender];
            break;
    }	
}

- (IBAction)AddPayee:(id)sender
{
	int count = [m_aPayeeItems count];
	
	[m_aPayeeItems addObject:@""];
	
	[payeesTableView reloadData];
	
	[payeesTableView editColumn:0 row:count withEvent:nil select:YES];
}

- (IBAction)DeletePayee:(id)sender
{
	NSInteger row = [payeesTableView selectedRow];
	
	if (row >= 0)
	{
		NSString *sPayee = [m_aPayeeItems objectAtIndex:row];
		
		std::string strPayee = [sPayee cStringUsingEncoding:NSUTF8StringEncoding];
		
		[m_aPayeeItems removeObjectAtIndex:row];
		
		m_Document.deletePayee(strPayee);
		
		[payeesTableView reloadData];
		
		[self setDocumentModified:TRUE];
	}
}

- (IBAction)AddCategory:(id)sender
{
	int count = [m_aCategoryItems count];
	
	[m_aCategoryItems addObject:@""];
	
	[categoriesTableView reloadData];
	
	[categoriesTableView editColumn:0 row:count withEvent:nil select:YES];
}

- (IBAction)DeleteCategory:(id)sender
{
	NSInteger row = [categoriesTableView selectedRow];
	
	if (row >= 0)
	{
		NSString *sCategory = [m_aCategoryItems objectAtIndex:row];
		
		std::string strCategory = [sCategory cStringUsingEncoding:NSUTF8StringEncoding];
		
		[m_aCategoryItems removeObjectAtIndex:row];
		
		m_Document.deleteCategory(strCategory);
		
		[categoriesTableView reloadData];
		
		[self setDocumentModified:TRUE];
	}
}

- (IBAction)AddScheduledTransaction:(id)sender
{
	NSDate *ndate1 = [scheduledDateCntl dateValue];
	NSCalendarDate *CalDate = [ndate1 dateWithCalendarFormat:0 timeZone:0];
	
	int nYear = [CalDate yearOfCommonEra];
	int nMonth = [CalDate monthOfYear];
	int nDay = [CalDate dayOfMonth];
	
	Date date1(nDay, nMonth, nYear);
	
	ScheduledTransaction newST;
	newST.setNextDate(date1);
	
	int schedTransIndex = m_Document.addScheduledTransaction(newST);
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];

	NSMutableDictionary *item = [[NSMutableDictionary alloc] init];
	
	std::string strSPayee = newST.getPayee();
	NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
	
	std::string strSCategory = newST.getCategory();
	NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
	
	NSString *sSAmount = [[valueFormatter currencyStringFromFixed:newST.getAmount()] retain];
	
	NSString *sSDate = [[dateFormatter stringFromDate:ndate1] retain];	
	
	[item setValue:[NSNumber numberWithInt:schedTransIndex] forKey:@"index"];
	[item setValue:sSPayee forKey:@"payee"];
	[item setValue:sSCategory forKey:@"category"];
	[item setValue:sSAmount forKey:@"amount"];
	[item setValue:sSDate forKey:@"nextdate"];
	[item setValue:[NSNumber numberWithBool:true] forKey:@"enabled"];
	
	[m_aScheduledTransactions addObject:item];
	
	[dateFormatter release];
	
	[scheduledTransactionsTableView reloadData];
	
	NSInteger row = schedTransIndex;
	
	[scheduledTransactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	[scheduledTransactionsTableView scrollRowToVisible:row];
	
	[window makeFirstResponder:scheduledPayee];
	
	[self setDocumentModified:TRUE];
}

- (IBAction)DeleteScheduledTransaction:(id)sender
{
	NSInteger row = [scheduledTransactionsTableView selectedRow];
	
	if (row >= 0)
	{
		[m_aScheduledTransactions removeObjectAtIndex:row];
		m_Document.deleteScheduledTransaction(row);
	}
	
	[scheduledTransactionsTableView reloadData];
	[self setDocumentModified:TRUE];
}

- (IBAction)AddGraph:(id)sender
{
	if (m_Document.getAccountCount() == 0)
	{
		NSRunAlertPanel(NSLocalizedString(@"No Accounts Exist", "No Accounts For Graph Error Title"),
						NSLocalizedString(@"You must have some accounts in the current document first before you can create a graph.", "No Accounts For Graph Error Message"),
						NSLocalizedString(@"OK", "OK Button"), nil, nil);
		return;
	}
	
	NSString *sNewGraph = NSLocalizedString(@"New Graph", "New Graph Text");
	std::string strNewGraph = [sNewGraph cStringUsingEncoding:NSUTF8StringEncoding];
	
	Graph newGraph;
	newGraph.setName(strNewGraph);
	newGraph.setAccount(0);
	
	Date currentDate;
	currentDate.Now();
	
	newGraph.setEndDate(currentDate);
	
	currentDate.DecrementMonths(1);
	newGraph.setStartDate(currentDate);
	
	int nGraphNum = m_Document.addGraph(newGraph);
	
	NSString *sGraphKey = [NSString stringWithFormat:@"g%d", nGraphNum];
	
	[indexBar addItem:@"graphs" key:sGraphKey title:sNewGraph item:nGraphNum action:@selector(graphSelected:) target:self type:3 rename:@selector(graphRenamed:) renameTarget:self];
	
	[self setDocumentModified:TRUE];
	
	[indexBar reloadData];
	
	[indexBar renameItem:sGraphKey];
}

// Payees/Categories/SchedTrans TableView Start

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	id result = @"";
	NSString *identifier = [aTableColumn identifier];
	
	if (aTableView == payeesTableView)
	{
		NSString *sPayee = [m_aPayeeItems objectAtIndex:rowIndex];
		
		if ([identifier isEqualToString:@"payee"])
		{		
			result = sPayee;
		}
	}
	else if (aTableView == categoriesTableView)
	{
		NSString *sCategory = [m_aCategoryItems objectAtIndex:rowIndex];
		
		if ([identifier isEqualToString:@"category"])
		{		
			result = sCategory;
		}
	}
	else if (aTableView == scheduledTransactionsTableView)
	{
		NSMutableDictionary *oObject = [m_aScheduledTransactions objectAtIndex:rowIndex];
		
		if ([identifier caseInsensitiveCompare:@"enabled"] == NSOrderedSame)
		{
			int nEnabled = [[oObject valueForKey:@"enabled"] intValue];
			return [NSNumber numberWithInt:nEnabled];
		}
		
		result = [oObject valueForKey:identifier];
	}
		
	return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	if (aTableView == payeesTableView)
		return [m_aPayeeItems count];
	else if (aTableView == categoriesTableView)
		return [m_aCategoryItems count];
	else if (aTableView == scheduledTransactionsTableView)
		return [m_aScheduledTransactions count];
	
	return 0;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
	NSString *identifier = [tableColumn identifier];
	
	if (rowIndex < 0)
		return;
	
	if (aTableView == scheduledTransactionsTableView)
	{
		if ([identifier isEqualToString:@"enabled"])
		{			
			NSMutableDictionary *nsSchedTrans = [m_aScheduledTransactions objectAtIndex:rowIndex];
			
			if (nsSchedTrans == nil)
				return;
			
			[nsSchedTrans setValue:[NSNumber numberWithBool:[object boolValue]] forKey:@"enabled"];
			
			ScheduledTransaction &oSchedTrans = m_Document.getScheduledTransaction(rowIndex);
			
			if ([object boolValue] == YES)
			{
				oSchedTrans.setEnabled(true);					
			}
			else
			{
				oSchedTrans.setEnabled(false);
			}
			
			[self setDocumentModified:TRUE];
		}			
	}
	else if (aTableView == payeesTableView)
	{
		NSString *sItem = [m_aPayeeItems objectAtIndex:rowIndex];
		
		if ([sItem length] == 0)
		{
			NSString *sNewItem = object;
			
			std::string strPayee = [sNewItem cStringUsingEncoding:NSUTF8StringEncoding];
		
			[m_aPayeeItems replaceObjectAtIndex:rowIndex withObject:sNewItem];
			
			m_Document.addPayee(strPayee);
			
			[payeesTableView reloadData];
			
			[self setDocumentModified:TRUE];
		}
	}
	else if (aTableView == categoriesTableView)
	{
		NSString *sItem = [m_aCategoryItems objectAtIndex:rowIndex];
		
		if ([sItem length] == 0)
		{
			NSString *sNewItem = object;
			
			std::string strCategory = [sNewItem cStringUsingEncoding:NSUTF8StringEncoding];
			
			[m_aCategoryItems replaceObjectAtIndex:rowIndex withObject:sNewItem];
			
			m_Document.addCategory(strCategory);
			
			[categoriesTableView reloadData];
			
			[self setDocumentModified:TRUE];
		}
	}
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	return NO;
}

// Payees/Categories TableView End

- (IBAction)NewFile:(id)sender
{
	if (m_Document.hasUnsavedChanges())
    {
		int choice = NSAlertDefaultReturn;
		
		NSString *title = NSLocalizedString(@"Do you want to save the changes you made in this document?", "Save Document Warning Title");
		
		choice = NSRunAlertPanel(title, NSLocalizedString(@"Your changes will be lost if you don't save them.", "Save Document Warning Message"),
								 NSLocalizedString(@"Save", "Save Button"),
								 NSLocalizedString(@"Don't Save", "Don't Save Button"),
								 NSLocalizedString(@"Cancel", "Cancel Button"));
		
		if (choice == NSAlertDefaultReturn) // Save file
		{
			[self SaveFile:sender];
		}
		else if (choice == NSAlertOtherReturn) // Cancel
		{
			return;
		}
    }
	
	[self setDocumentModified:FALSE];
	m_DocumentFile = "";
	
	m_pAccount = 0;
	
	m_Document.clear();
	
	[self setWindowTitleWithDocName:nil];
	
	[self buildIndexTree];

	[self buildSchedTransList];
    
    // calling this don't do anything, as we don't have an account
	[self refreshLibraryItems];
    
    // so has a hack to clear the transactions list, let's do this:
    TransactionsController* pTC = [TransactionsController sharedInterface];
	[pTC showTransactionsForAccount:Nil];
}

- (IBAction)OpenFile:(id)sender
{
	if (m_Document.hasUnsavedChanges())
    {
		int choice = NSAlertDefaultReturn;
		
		NSString *title = NSLocalizedString(@"Do you want to save the changes you made in this document?", "Save Document Warning Title");
		
		choice = NSRunAlertPanel(title, NSLocalizedString(@"Your changes will be lost if you don't save them.", "Save Document Warning Message"),
								 NSLocalizedString(@"Save", "Save Button"),
								 NSLocalizedString(@"Don't Save", "Don't Save Button"),
								 NSLocalizedString(@"Cancel", "Cancel Button"));
		
		if (choice == NSAlertDefaultReturn) // Save file
		{
			[self SaveFile:sender];
		}
		else if (choice == NSAlertOtherReturn) // Cancel
		{
			return;
		}
    }
	
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	NSString *fileToOpen;
	std::string strFile = "";
	[oPanel setAllowsMultipleSelection: NO];
	[oPanel setResolvesAliases: YES];
	[oPanel setTitle:NSLocalizedString(@"Open Document", "Open Document Title")];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"stash", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
        NSURL* urlToOpen = [[oPanel URL] retain];
		fileToOpen = [urlToOpen path];
		strFile = [fileToOpen cStringUsingEncoding: NSUTF8StringEncoding];
		
		if ([self OpenFileAt:strFile] == false)
		{
			NSAlert *alert = [[NSAlert alloc] init];
			[alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Could not open file: \"%@\".", "Open Document Error Message"), fileToOpen]];
			[alert setInformativeText:NSLocalizedString(@"There was a problem opening the Document file.", "Open Document Error Text")];
			[alert setAlertStyle: NSWarningAlertStyle];
			[alert addButtonWithTitle:NSLocalizedString(@"OK", "OK Button")];
			
			[alert runModal];
			[alert release];
			return;
		}
		
		[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:fileToOpen]];
		
		[self setWindowTitleWithDocName:fileToOpen];
		
		m_DocumentFile = strFile;
		[self setDocumentModified:FALSE];
		
		m_pAccount = 0;
		
		[self buildIndexTree];
		[self refreshLibraryItems];
		
		[self calculateAndShowScheduled];
	}
}

- (IBAction)SaveFile:(id)sender
{
	if (m_DocumentFile.empty())
	{
		[self SaveFileAs:sender];
	}
	else
	{
		if ([self SaveFileTo:m_DocumentFile] == true)
		{
			[self setDocumentModified:FALSE];
		}
	}
}

- (IBAction)SaveFileAs:(id)sender
{
	NSSavePanel *sPanel = [NSSavePanel savePanel];
	NSString *fileToSave;
	std::string strFile = "";
	
	[sPanel setTitle:NSLocalizedString(@"Save Document", "Save Document Title")];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"stash", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
        NSURL* urlToSave = [[sPanel URL] retain];
        fileToSave = [urlToSave path];
		strFile = [fileToSave cStringUsingEncoding: NSUTF8StringEncoding];
		
		if ([self SaveFileTo:strFile] == false)
		{
			NSAlert *alert = [[NSAlert alloc] init];
			[alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Could not save to file: \"%@\".", "Save Document Error Message"), fileToSave]];
			[alert setInformativeText:NSLocalizedString(@"There was a problem saving the Document to the file.", "Save Document Error Text")];
			[alert setAlertStyle: NSWarningAlertStyle];
			[alert addButtonWithTitle:NSLocalizedString(@"OK", "OK Button")];
			
			[alert runModal];
			[alert release];
		}
		else
		{
			[self setWindowTitleWithDocName:fileToSave];
			
			[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:fileToSave]];
			
			m_DocumentFile = strFile;
			[self setDocumentModified:FALSE];
		}
	}
}

- (bool)OpenFileAt:(std::string)path
{
	std::fstream fileStream(path.c_str(), std::ios::in | std::ios::binary);
	
	if (!fileStream)
	{
		return false;
	}
	
	// hacky, but...
	// this will be set to true if Load() returns false due to a future file format
	// version we don't know about...
	bool isFutureFileVersion = false;
	
	if (!m_Document.Load(fileStream, isFutureFileVersion))
	{
		fileStream.close();
		
		NSRunAlertPanel(@"Unrecognised Stash file format version",
						@"The file format of the Stash document you are trying to open is from a newer version of Stash which this version of Stash does not know about or understand, so it can not open this document. Please use a newer version of Stash to open this document.",
						@"OK", nil, nil);
		
		return false;
	}
	
	fileStream.close();
	
	return true;	
}

- (bool)SaveFileTo:(std::string)path
{
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"GeneralCreateBackupOnSave"] == YES)
	{
		std::string strPathBackup = path;
		strPathBackup += ".bak";
		
		rename(path.c_str(), strPathBackup.c_str());
	}
	
	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	
	if (!fileStream)
	{
		return false;
	}
	
	m_Document.Store(fileStream);
	
	fileStream.close();
	
	return true;
}

- (void)calculateAndShowScheduled
{
	NSMutableArray *array = [[NSMutableArray alloc] init];
	
	std::vector<ScheduledTransaction>::iterator it = m_Document.SchedTransBegin();
	
	int schedTransIndex = 0;
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	
	Date today;
	
	for (; it != m_Document.SchedTransEnd(); ++it, schedTransIndex++)
	{
		if (it->isEnabled() && it->getNextDate() <= today)
		{
			NSMutableDictionary *item = [[NSMutableDictionary alloc] init];
			
			std::string strSPayee = it->getPayee();
			NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
			
			std::string strSDesc = it->getDescription();
			NSString *sSDesc = [[NSString alloc] initWithUTF8String:strSDesc.c_str()];
			
			NSString *sSAmount = [[valueFormatter currencyStringFromFixed:it->getAmount()] retain];
			
			NSDate *date = convertToNSDate(it->getNextDate());
			NSString *sSDate = [[dateFormatter stringFromDate:date] retain];
			
			int nAccount = it->getAccount();
			
			if (nAccount >= 0)
			{
				Account &oAccount = m_Document.getAccount(nAccount);
				std::string strAccount = oAccount.getName();
				NSString *sAccount = [[NSString alloc] initWithUTF8String:strAccount.c_str()];
				
				[item setValue:[NSNumber numberWithInt:schedTransIndex] forKey:@"index"];
				[item setValue:sSPayee forKey:@"payee"];
				[item setValue:sSDesc forKey:@"desc"];
				[item setValue:sSAmount forKey:@"amount"];
				[item setValue:sSDate forKey:@"date"];
				[item setValue:sAccount forKey:@"account"];
				
				[array addObject:item];
			}
            
            [item release];
		}
	}
	
	[dateFormatter release];
	
	if ([array count] > 0)
	{
		DueScheduledTransactionsController *dueSchedController = [[DueScheduledTransactionsController alloc] initWnd:self withArray:array];
	
		[dueSchedController showWindow:self];
	}
	
	[array release];
}

- (void)AddDueScheduledTransaction:(int)index
{
	Date today;
	
	ScheduledTransaction &schedTrans = m_Document.getScheduledTransaction(index);
	
	Transaction newTransaction(schedTrans.getDescription(), schedTrans.getPayee(), schedTrans.getCategory(), schedTrans.getAmount(), today);
	
	newTransaction.setType(schedTrans.getType());
	newTransaction.setCleared(true);
	
	Account &oAccount = m_Document.getAccount(schedTrans.getAccount());
	
	oAccount.addTransaction(newTransaction);
	
	TransactionsController* pTC = [TransactionsController sharedInterface];	
	[pTC buildTransactionsTree];
	
	schedTrans.AdvanceNextDate();
    
    // update the index view so the accounts show the correct new final balance
    [self buildIndexTree];
	
	[self setDocumentModified:TRUE];
}

- (void)SkipDueScheduledTransaction:(int)index
{
	ScheduledTransaction &schedTrans = m_Document.getScheduledTransaction(index);
	
	schedTrans.AdvanceNextDate();
	
	[self setDocumentModified:TRUE];
}

- (IBAction)ImportQIF:(id)sender
{
	if (!m_pAccount)
	{
		NSRunAlertPanel(NSLocalizedString(@"No Active Account", "Import QIF -> No Active Account Warning Title"),
						NSLocalizedString(@"You must have an active account in order to import QIF file data.", "Import QIF -> No Active Account Warning Message"),
						NSLocalizedString(@"OK", "OK Button"), nil, nil);
		return;
	}
	
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	NSString *fileToOpen;
	std::string strFile = "";
	[oPanel setAllowsMultipleSelection:NO];
	[oPanel setResolvesAliases:YES];
	[oPanel setTitle:NSLocalizedString(@"Import QIF file", "Import QIF Title")];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"qif", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
        NSURL* urlToOpen = [[oPanel URL] retain];
        fileToOpen = [urlToOpen path];
		strFile = [fileToOpen cStringUsingEncoding: NSUTF8StringEncoding];
		
		std::string strDateSample = "";
		NSString *sDateSample = @"";
		
		if (getDateFormatSampleFromQIFFile(strFile, strDateSample))
		{
			sDateSample = [[NSString alloc] initWithUTF8String:strDateSample.c_str()];
		}
		
		ImportQIFController *importQIFController = [[ImportQIFController alloc] initWnd:self withFile:fileToOpen sampleFormat:sDateSample];
		[importQIFController showWindow:self];
	}
}

- (void)importQIFConfirmed:(ImportQIFController *)importQIFController
{
	int nDateFormat = [importQIFController dateFormat];
	Date::DateStringFormat dateFormat = static_cast<Date::DateStringFormat>(nDateFormat);
	
	char cSeparator = [importQIFController separator];
	
	BOOL bMarkCleared = [importQIFController markAsCleared];
	
	NSString *sFile = [importQIFController file];
		
	std::string strFile = [sFile cStringUsingEncoding: NSUTF8StringEncoding];
	
	[importQIFController release];
	
	bool bMC = true;
	if (bMarkCleared == NO)
		bMC = false;
	
	if (importQIFFileToAccount(m_pAccount, strFile, dateFormat, cSeparator, bMC))
	{
		TransactionsController* pTC = [TransactionsController sharedInterface];
		[pTC buildTransactionsTree];
		
		[self setDocumentModified:TRUE];
	}
	
	// update the balance in the IndexBar
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	fixed accountBalance = m_pAccount->getBalance(true);	
	
	NSString *sBalance = [[valueFormatter currencyStringFromFixed:accountBalance] retain];
	
	NSString *itemKey = [indexBar getSelectedItemKey];
	
	if (itemKey)
	{
		[indexBar updateAmount:itemKey amount:sBalance];
		
		[indexBar reloadData];
	}
}

- (IBAction)ImportOFX:(id)sender
{
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	NSString *fileToOpen;
	std::string strFile = "";
	[oPanel setAllowsMultipleSelection:NO];
	[oPanel setResolvesAliases:YES];
	[oPanel setTitle:NSLocalizedString(@"Select OFX file to Import", "Import OFX Title")];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"ofx", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
        NSURL* urlToOpen = [[oPanel URL] retain];
        fileToOpen = [urlToOpen path];
		strFile = [fileToOpen cStringUsingEncoding: NSUTF8StringEncoding];
		
		OFXData dataItem;
		
		if (!importOFXFile(strFile, dataItem))
		{
			NSRunAlertPanel(NSLocalizedString(@"Couldn't Import OFX File", "Import OFX Error Title"),
							NSLocalizedString(@"Couldn't import the selected OFX file. Check it is a valid OFX 1.x/2.x file.", "Import OFX Error Message"),
							NSLocalizedString(@"OK", "OK Button"), nil, nil);
			return;	
		}
		
		int numAccounts = dataItem.getResponseCount();
		
		if (numAccounts == 0)
		{
			NSRunAlertPanel(NSLocalizedString(@"No data in file", "Import OFX No Data Title"),
							NSLocalizedString(@"Stash couldn't find any account/transaction data in the OFX file.", "Import OFX No Data Message"),
							NSLocalizedString(@"OK", "OK Button"), nil, nil);
			return;
		}
		
		bool bFoundReverse = false;
		BOOL bReverseTransactions = NO;
		
		ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
		
		NSMutableArray *aAccountsData = [[NSMutableArray alloc] init];
		
		OFXStTrResIt sTRIt = dataItem.begin();
		
		for (; sTRIt != dataItem.end(); ++sTRIt)
		{
			OFXStatementResponse &stResp = (*sTRIt).getStatementResponse();
			
			NSMutableDictionary *importItem = [[NSMutableDictionary alloc] init];
			
			const OFXBankAccount &bankAccount = stResp.getAccount();
			
			// try and work out if transactions need to be reversed
			int nMaxLook = stResp.getTransactionCount() - 1;
			
			if (!bFoundReverse && nMaxLook > 1)
			{
				const Date &dtStart = stResp.getTransaction(0).getDate();
				const Date &dtEnd = stResp.getTransaction(nMaxLook).getDate();
				
				if (dtStart > dtEnd)
				{
					bFoundReverse = true;
					bReverseTransactions = YES;
				}				
			}
			
			NSString *sAccountNumber = [[NSString alloc] initWithUTF8String:bankAccount.getAccountID().c_str()];		
			NSString *sTransactionCount = [NSString stringWithFormat:@"%i", stResp.getTransactionCount()];
			
			fixed balance = stResp.getBalance();
			
			NSString *sBalance = [[valueFormatter currencyStringFromFixed:balance] retain];
						
			[importItem setValue:sAccountNumber forKey:@"accno"];
			[importItem setValue:sTransactionCount forKey:@"transcount"];
			[importItem setValue:sBalance forKey:@"balance"];
			
			[importItem setObject:[NSNumber numberWithBool:YES] forKey:@"import"];
			[importItem setObject:[NSNumber numberWithInt:1] forKey:@"importType"];
			[importItem setValue:sAccountNumber forKey:@"account"];
			[importItem setValue:sAccountNumber forKey:@"newAccName"];
			[importItem setObject:[NSNumber numberWithInt:0] forKey:@"newAccType"];			
			
			[aAccountsData addObject:importItem];			
		}
		
		NSMutableArray *aExistingAccounts = [[NSMutableArray alloc] init];
		
		std::vector<Account>::iterator it = m_Document.AccountBegin();
		
		for (; it != m_Document.AccountEnd(); ++it)
		{
			std::string strName = it->getName();
			NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
			[aExistingAccounts addObject:sName];
			
			[sName release];
		}
		
		if (!importOFXController)
			importOFXController = [[ImportOFXController alloc] initWithOFXData:aAccountsData existingAccounts:aExistingAccounts file:strFile];
		
		[importOFXController showImportOFXWindow:window reverse:bReverseTransactions];
	}
}

- (void)importOFXFileWithController:(ImportOFXController*)controller reverseTransactions:(bool)reverse cleared:(bool)cleared ignoreExisting:(bool)ignoreExisting
{
	NSString *sFileName = [controller getFilename];
	std::string strFilename = [sFileName cStringUsingEncoding: NSUTF8StringEncoding];
	
	OFXData dataItem;
	
	if (!importOFXFile(strFilename, dataItem))
	{
		NSRunAlertPanel(NSLocalizedString(@"Couldn't Import OFX file", "Import OFX Error Title"),
						NSLocalizedString(@"Couldn't import the selected OFX file. Check it is a valid OFX 1.x/2.x file.", "Import OFX Error Message"),
						NSLocalizedString(@"OK", "OK Button"), nil, nil);
		
		[importOFXController release];
		importOFXController = 0;
		
		return;
	}
	
	NSArray *settings = [controller getSettings];
	
	int numAccountSettings = [settings count];
	int numAccountsInFile = dataItem.getResponseCount();
	
	if (numAccountSettings != numAccountsInFile)
	{
		// something's gone wrong
		NSRunAlertPanel(NSLocalizedString(@"Import data inconcistency", "Import OFX Data Inconcistency Title"),
						NSLocalizedString(@"The Import data could not be processed correctly.", "Import OFX Data Inconcistency Message"),
						NSLocalizedString(@"OK", "OK Button"), nil, nil);
			
		[importOFXController release];
		importOFXController = 0;
		return;
	}
	
	// remember which IndexBar item is currently selected
	NSString *selectedKey = [[indexBar getSelectedItemKey] retain];
	
	for (int i = 0; i < numAccountSettings; i++)
	{
		NSMutableDictionary *accountSettings = [settings objectAtIndex:i];
		
		if ([[accountSettings objectForKey:@"import"] boolValue] == NO)
			 continue;
							  
		OFXStatementTransactionResponse &response = dataItem.getResponse(i);		
		OFXStatementResponse &stResp = response.getStatementResponse();
		
		int importType = [[accountSettings objectForKey:@"importType"] intValue];
		
		if (importType == 0) // existing Account
		{
			int existingAccountIndex = [[accountSettings objectForKey:@"existingAccount"] intValue];
			Account &existingAccount = m_Document.getAccount(existingAccountIndex);
			
			importOFXStatementIntoAccount(existingAccount, stResp, reverse, cleared, ignoreExisting);
		}
		else // new Account
		{
			NSString *sAccountName = [accountSettings valueForKey:@"newAccName"];
			std::string strAccountName = [sAccountName cStringUsingEncoding:NSUTF8StringEncoding];
			
			int accountType = [[accountSettings objectForKey:@"newAccType"] intValue];
			
			AccountType eType = static_cast<AccountType>(accountType);
			
			Account newAccount;
			newAccount.setName(strAccountName);
			newAccount.setType(eType);
			
			importOFXStatementIntoAccount(newAccount, stResp, reverse, cleared, ignoreExisting);
			
			m_Document.addAccount(newAccount);			
		}		
	}
	
	[self setDocumentModified:TRUE];
	
	[self buildIndexTree];
	
	if (selectedKey != nil)
		[indexBar selectItem:selectedKey];
	
	[importOFXController release];
	importOFXController = 0;
}

- (void)deleteImportOFXController
{
	if (importOFXController)
	{
		[importOFXController release];
		importOFXController = 0;
	}
}

- (IBAction)ExportOFX:(id)sender
{
	NSSavePanel *sPanel = [NSSavePanel savePanel];
	NSString *fileToSave;
	std::string strFile = "";
	[sPanel setTitle:NSLocalizedString(@"Select OFX file to Export to", "Export OFX Title")];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"ofx", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
        NSURL* urlToSave = [[sPanel URL] retain];
        fileToSave = [urlToSave path];
		strFile = [fileToSave cStringUsingEncoding: NSUTF8StringEncoding];
		
		NSMutableArray *aExistingAccounts = [[NSMutableArray alloc] init];
		
		std::vector<Account>::iterator it = m_Document.AccountBegin();
		
		int accountCount = 0;
		
		for (; it != m_Document.AccountEnd(); ++it)
		{
			NSMutableDictionary *accountItem = [[NSMutableDictionary alloc] init];
			std::string strName = it->getName();
			NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
			
			[accountItem setValue:sName forKey:@"account"];
			[accountItem setObject:[NSNumber numberWithBool:YES] forKey:@"export"];
			[accountItem setObject:[NSNumber numberWithInt:accountCount++] forKey:@"num"];			
			
			[aExistingAccounts addObject:accountItem];
			
			[sName release];
		}
		
		if (!exportOFXController)
			exportOFXController = [[ExportOFXController alloc] initWithAccountData:aExistingAccounts file:strFile];
		
		[exportOFXController showExportOFXWindow:window];
	}
}

- (void)exportOFXFileWithController:(ExportOFXController*)controller xmlOFX:(bool)xml
{
	NSString *sFileName = [controller getFilename];
	std::string strFilename = [sFileName cStringUsingEncoding:NSUTF8StringEncoding];
	
	OFXData dataItem;
	
	NSArray *accounts = [controller getAccounts];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	NSString *currencyCode = [numberFormatter currencyCode];
	std::string strCurrencyCode = [currencyCode cStringUsingEncoding:NSUTF8StringEncoding];
	
	NSMutableDictionary *accountItem = 0;
	
	for (accountItem in accounts)
	{
		int accountIndex = [[accountItem objectForKey:@"num"] intValue];
		
		BOOL bExport = [[accountItem objectForKey:@"export"] boolValue];
		
		if (bExport)
		{
			Account &account = m_Document.getAccount(accountIndex);
			
			OFXStatementTransactionResponse newResponseItem;
			newResponseItem.getStatementResponse().setCurrencyCode(strCurrencyCode);
			
			newResponseItem.addOFXTransactionsForAccount(account);
			
			dataItem.addStatementTransactionResponse(newResponseItem);			
		}
	}
	
	[numberFormatter release];
	
	dataItem.exportDataToFile(strFilename, xml);
}

- (void)deleteExportOFXController
{
	if (exportOFXController)
	{
		[exportOFXController release];
		exportOFXController = 0;
	}
}

- (IBAction)ExportQIF:(id)sender
{
	NSSavePanel *sPanel = [NSSavePanel savePanel];
	NSString *fileToSave;
	std::string strFile = "";
	
	[sPanel setTitle:NSLocalizedString(@"Export to QIF file", "Export QIF Title")];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"qif", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
        NSURL* urlToSave = [[sPanel URL] retain];
        fileToSave = [urlToSave path];
		strFile = [fileToSave cStringUsingEncoding: NSUTF8StringEncoding];
		
		exportAccountToQIFFile(m_pAccount, strFile, Date::UK);
	}	
}

- (IBAction)showPreferencesWindow:(id)sender
{
	NSWindow *window1 = [prefController window];
    if (![window1 isVisible])
        [window1 center];
	
    [window1 makeKeyAndOrderFront:self];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
	
	if (action == @selector(showRecentTransactions:))
    {
        [menuItem setState:ShowTransactionsViewType == RECENT ? NSOnState : NSOffState];
        return YES;
    }
	if (action == @selector(showAllTransactionsThisYear:))
    {
        [menuItem setState:ShowTransactionsViewType == ALL_THIS_YEAR ? NSOnState : NSOffState];
        return YES;
    }
	if (action == @selector(showAllTransactions:))
    {
        [menuItem setState:ShowTransactionsViewType == ALL ? NSOnState : NSOffState];
        return YES;
    }
	
	if (action == @selector(MakeTransfer:))
	{
		if (m_Document.getAccountCount() < 2)
			return NO;
	}
	
	return YES;
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem
{
    NSString *ident = [toolbarItem itemIdentifier];
    
    if ([ident isEqualToString:TOOLBAR_VIEWTYPE])
        return nViewType == 0;
	
	if ([ident isEqualToString:TOOLBAR_MAKETRANSFER])
	{
		if (m_Document.getAccountCount() < 2)
			return NO;
	}
	
	return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	if (m_Document.hasUnsavedChanges())
    {
		int choice = NSAlertDefaultReturn;
		
		NSString *title = NSLocalizedString(@"Do you want to save the changes you made in this document?", "Save Document Warning Title");
		
		choice = NSRunAlertPanel(title, NSLocalizedString(@"Your changes will be lost if you don't save them.", "Save Document Warning Message"),
								 NSLocalizedString(@"Save", "Save Button"),
								 NSLocalizedString(@"Don't Save", "Don't Save Button"),
								 NSLocalizedString(@"Cancel", "Cancel Button"));
		
		if (choice == NSAlertDefaultReturn) // Save file
		{
			[self SaveFile:sender];
			return NSTerminateNow;
		}
		else if (choice == NSAlertOtherReturn) // Cancel
		{
			return NSTerminateLater;
		}
    }
	
    return NSTerminateNow;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	// save the window size and position
	NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
	
	[defs setFloat:[window frame].origin.x forKey:@"MainWndPosX"];
	[defs setFloat:[window frame].origin.y forKey:@"MainWndPosY"];
	[defs setFloat:[window frame].size.width forKey:@"MainWndWidth"];
	[defs setFloat:[window frame].size.height forKey:@"MainWndHeight"];
	
	TransactionsController* pTController = [TransactionsController sharedInterface];
	[pTController cleanUp];
	
	// Save the splitter positions
	[indexBarSplitView saveLayoutToDefault:@"indexSplitter"];
	[scheduledverticalSplitView saveLayoutToDefault:@"schedtransSplitter"];
	
	[m_aPayeeItems release];
	[m_aCategoryItems release];
	[m_aScheduledTransactions release];
	
	GraphController* pGC = [GraphController sharedInterface];
	[pGC cleanUp];
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	if (!m_HasFinishedLoading)
	{
		// App hasn't finished loading, and Cocoa gets sent this before the app's finished loading
		// (via the command line), so we have to do things like this, otherwise things are out of sync
		
		m_sPendingOpenFile = filename;
		return NO;
	}
	
	if (m_Document.hasUnsavedChanges())
    {
		int choice = NSAlertDefaultReturn;
		
        NSString * message = NSLocalizedString(@"You have unsaved changes in this document. Are you sure you want to open another document?", "Replace Document Warning Message");
		
		choice = NSRunAlertPanel(NSLocalizedString(@"Replace current document?", "Replace Document Warning Title"),
								 message, NSLocalizedString(@"Replace", "Replace Button"),
								 NSLocalizedString(@"Cancel", "Cancel Button"), nil);
		
		if (choice != NSAlertDefaultReturn)
			return NO;
    }
	
	std::string strFile = [filename cStringUsingEncoding: NSUTF8StringEncoding];
	
	if ([self OpenFileAt:strFile] == false)
	{
		return NO;
	}
	
	[self setWindowTitleWithDocName:filename];
	
	[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:filename]];
	
	m_DocumentFile = strFile;
	[self setDocumentModified:FALSE];
	
	[self buildIndexTree];
	[self refreshLibraryItems];
	
	[self calculateAndShowScheduled];

	return YES;
}

- (IBAction)gotoWebsite:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://wiki.github.com/ppearson/Stash"]];
}

- (IBAction)reportBug:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://stash.lighthouseapp.com/"]];
}

- (void)setDocumentModified:(BOOL)modified
{
	m_Document.setUnsavedChanges(modified);
	[[self window] setDocumentEdited:modified];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)application
{
    return YES;
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	[NSApp terminate:self];
}

@end
