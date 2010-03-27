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
#import "NSDateEx.h"

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

@synthesize window, ShowTransactionsViewType;

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
		
		transactionsTableHeaderMenu = [[NSMenu alloc] initWithTitle:@"Menu"];
		
		NSNotificationCenter *nc;
		nc = [NSNotificationCenter defaultCenter];
		[nc addObserver:self selector:@selector(handleTransactionsSettingsUpdate:) name:@"TransactionsSettingsUpdate" object:nil];
		[nc addObserver:self selector:@selector(handleGraphSettingsUpdate:) name:@"GraphSettingsUpdate" object:nil];
		
		m_HasFinishedLoading = false;
		m_sPendingOpenFile = nil;
		
		[NSApp setDelegate: self];
	}
	
	return self;
}

- (void)dealloc
{
	[prefController release];
	[transactionsTableHeaderMenu release];
	
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
	
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"TransactionsScrollToLatest"];
	[defaultValues setObject:[NSNumber numberWithBool:NO] forKey:@"TransactionsNegAmountsRed"];
	[defaultValues setObject:[NSNumber numberWithBool:YES] forKey:@"TransactionsNegBalancesRed"];
	[defaultValues setObject:[NSNumber numberWithInt:30] forKey:@"TransactionsRecentDuration"];
	
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
	
	NSDictionary *transactionColumnOrder = [NSArray arrayWithObjects:@"Cleared", @"Date", @"Payee", @"Category", @"Description", @"Amount", @"Balance", nil];
	
	[defaultValues setObject:visibleTransactionColumns forKey:@"VisibleTransactionColumns"];
	[defaultValues setObject:transactionColumnSizes forKey:@"TransactionColumnSizes"];
	[defaultValues setObject:transactionColumnOrder forKey:@"TransactionColumnOrder"];
		
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
}

- (void)awakeFromNib
{
	m_bEditing = false;	
	m_SelectedTransaction = 0;
	
	[window setDelegate:self];
	
	[indexView setFrameSize:[indexViewPlaceholder frame].size];
	[indexViewPlaceholder addSubview:indexView];
	
	[vTransactionsView setFrameSize:[contentViewPlaceholder frame].size];
	
	[contentViewPlaceholder addSubview:vTransactionsView];
	contentView = vTransactionsView;
	
	nViewType = 0;
	
	NSToolbar * toolbar = [[NSToolbar alloc] initWithIdentifier:@"Toolbar"];
    [toolbar setDelegate:self];
    [toolbar setAllowsUserCustomization:NO];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    [toolbar setSizeMode:NSToolbarSizeModeRegular];
    [[self window] setToolbar:toolbar];
    [toolbar release];
	
	// Load the splitter positions
	[indexBarSplitView loadLayoutFromDefault:@"indexSplitter"];
	[transactionsverticalSplitView loadLayoutFromDefault:@"transactionsSplitter"];
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
	
	int nTransactionsViewType = [[defs objectForKey:@"TransactionsViewType"] intValue];
	
	if (nTransactionsViewType >= 0 && nTransactionsViewType < 3)
	{
		self.ShowTransactionsViewType = nTransactionsViewType;
	}
	else
	{
		self.ShowTransactionsViewType = RECENT;
	}
	
	m_aTransactionItems = [[NSMutableArray alloc] init];
	m_aPayeeItems = [[NSMutableArray alloc] init];
	m_aCategoryItems = [[NSMutableArray alloc] init];
	m_aScheduledTransactions = [[NSMutableArray alloc] init];
	m_aGraphItems = [[NSMutableArray alloc] init];
	
	// Load Transactions view OutlineView column sizes
	
	NSDictionary *visibleTransactionColumns = [[NSUserDefaults standardUserDefaults] objectForKey:@"VisibleTransactionColumns"];
	NSDictionary *transactionColumnSizes = [[NSUserDefaults standardUserDefaults] objectForKey:@"TransactionColumnSizes"];
	NSMutableArray *transactionColumnOrder = [[NSUserDefaults standardUserDefaults] objectForKey:@"TransactionColumnOrder"];
		
	[[transactionsTableView headerView] setMenu:transactionsTableHeaderMenu];
	
	int nMenuIndex = 0;
	for (NSTableColumn *tc in [[transactionsTableView tableColumns] reverseObjectEnumerator])
	{
		NSMenuItem *subItem = [transactionsTableHeaderMenu insertItemWithTitle:[[tc headerCell] stringValue] action:@selector(transactionTableColumnMenu:) keyEquivalent:@"" atIndex:nMenuIndex];
		
		[subItem setTarget:self];
		[subItem setRepresentedObject:tc];
		
		float fWidth = [[transactionColumnSizes objectForKey:[tc identifier]] floatValue];
		
		if (fWidth > 0.0)
		{
			[tc setWidth:fWidth];
		}
		
		BOOL bIsVisible = [[visibleTransactionColumns objectForKey:[tc identifier]] boolValue];
		
		if (!bIsVisible)
		{
			[subItem setState:NSOffState];
			[tc setHidden:TRUE];
		}
		else
		{
			[subItem setState:NSOnState];
		}
		
		[[tc headerCell] setMenu:transactionsTableHeaderMenu];
	}
	
	int nCol = 0;
	for (NSString *sItem in transactionColumnOrder)
	{
		if ([sItem isEqualToString:@"Reconciled"])
			sItem = @"Cleared";
		
		[transactionsTableView moveColumn:[transactionsTableView columnWithIdentifier:sItem] toColumn:nCol++];
	}
	
	NSDate *date1 = [NSDate date];
	[transactionsDateCntl setDateValue:date1];
	[scheduledDateCntl setDateValue:date1];
	
	[transactionsType removeAllItems];
	
	[transactionsType addItemWithTitle:NSLocalizedString(@"None", "Transaction Type -> None")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Deposit", "Transaction Type -> Deposit")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Withdrawal", "Transaction Type -> Withdrawal")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Transfer", "Transaction Type -> Transfer")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Standing Order", "Transaction Type -> Standing Order")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Direct Debit", "Transaction Type -> Direct Debit")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Point Of Sale", "Transaction Type -> Point Of Sale")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Charge", "Transaction Type -> Charge")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"ATM", "Transaction Type -> ATM")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Check", "Transaction Type -> Check")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Credit", "Transaction Type -> Credit")];
	[transactionsType addItemWithTitle:NSLocalizedString(@"Debit", "Transaction Type -> Debit")];
		
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
	
	[graphType removeAllItems];
	
	[graphType addItemWithTitle:NSLocalizedString(@"Expense Categories", "Graph Pane -> Type -> Expense Categories")];
	[graphType addItemWithTitle:NSLocalizedString(@"Expense Payees", "Graph Pane -> Type -> Expense Payees")];
	[graphType addItemWithTitle:NSLocalizedString(@"Deposit Categories", "Graph Pane -> Type -> Deposit Categories")];
	[graphType addItemWithTitle:NSLocalizedString(@"Deposit Payees", "Graph Pane -> Type -> Deposit Payees")];
	
	[graphItemTypes removeAllItems];

	[graphItemTypes addItemWithTitle:NSLocalizedString(@"All Items", "Graph Pane -> Items Type -> All Items")];
	[graphItemTypes addItemWithTitle:NSLocalizedString(@"All Items excluding:", "Graph Pane -> Items Type -> All Items excluding:")];
	[graphItemTypes addItemWithTitle:NSLocalizedString(@"Only Specified:", "Graph Pane -> Items Type -> Only Specified:")];
	
	[transactionsType selectItemAtIndex:0];
	
	[addTransaction setToolTip:NSLocalizedString(@"Add Transaction", "Transactions Pane -> Add Transaction Button Tooltip")];
	[deleteTransaction setToolTip:NSLocalizedString(@"Delete Transaction", "Transactions Pane -> Delete Transaction Button Tooltip")];
	[splitTransaction setToolTip:NSLocalizedString(@"Split Transaction", "Transactions Pane -> Split Transaction Button Tooltip")];
	[moveDown setToolTip:NSLocalizedString(@"Move Down", "Transactions Pane -> Move Down Button Tooltip")];
	[moveUp setToolTip:NSLocalizedString(@"Move Up", "Transactions Pane -> Move Up Button Tooltip")];
	[refresh setToolTip:NSLocalizedString(@"Refresh", "Transactions Pane -> Refresh Button Tooltip")];
	
	[deleteTransaction setEnabled:NO];
	[splitTransaction setEnabled:NO];
	[moveUp setEnabled:NO];
	[moveDown setEnabled:NO];
	
	[deleteScheduled setEnabled:NO];
	
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(TransactionSelectionDidChange:) name:NSOutlineViewSelectionDidChangeNotification object:transactionsTableView];
	[nc addObserver:self selector:@selector(ScheduledSelectionDidChange:) name:NSTableViewSelectionDidChangeNotification object:scheduledTransactionsTableView];
	
	[transactionsTableView setDelegate:self];
	[transactionsTableView setAutoresizesOutlineColumn:NO];
	[payeesTableView setDelegate:self];	
	[categoriesTableView setDelegate:self];
	[scheduledTransactionsTableView setDelegate:self];
	[graphItemsTableView setDelegate:self];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)ident willBeInsertedIntoToolbar:(BOOL)flag
{
    NSToolbarItem * item = [[NSToolbarItem alloc] initWithItemIdentifier:ident];
	
    if ([ident isEqualToString:TOOLBAR_ADDACCOUNT])
    {
        [item setLabel:NSLocalizedString(@"Add Account", "Main Toolbar -> Add Account")];
        [item setImage:[NSImage imageNamed:@"add_account.png"]];
        [item setTarget:self];
        [item setAction:@selector(AddAccount:)];
        [item setAutovalidates:NO];
    }
    else if ([ident isEqualToString:TOOLBAR_ADDGRAPH])
    {
        [item setLabel:NSLocalizedString(@"Add Graph", "Main Toolbar -> Add Graph")];
        [item setImage:[NSImage imageNamed:@"add_graph.png"]];
        [item setTarget:self];
        [item setAction:@selector(AddGraph:)];
        [item setAutovalidates:NO];
    }
	else if ([ident isEqualToString:TOOLBAR_MAKETRANSFER])
    {
        [item setLabel:NSLocalizedString(@"Make Transfer", "Main Toolbar -> Make Transfer")];
        [item setImage:[NSImage imageNamed:@"make_transfer.png"]];
        [item setTarget:self];
        [item setAction:@selector(MakeTransfer:)];
        [item setAutovalidates:YES];
    }
	else if ([ident isEqualToString: TOOLBAR_VIEWTYPE])
    {
        ToolbarItemEx *groupItem = [[ToolbarItemEx alloc] initWithItemIdentifier:ident];
        
        NSSegmentedControl *segmentedControl = [[NSSegmentedControl alloc] initWithFrame:NSZeroRect];
        [segmentedControl setCell:[[[NSSegmentedCell alloc] init] autorelease]];
        [groupItem setView:segmentedControl];
        NSSegmentedCell *segmentedCell = (NSSegmentedCell *)[segmentedControl cell];
        
        [segmentedControl setSegmentCount:3];
        [segmentedCell setTrackingMode:NSSegmentSwitchTrackingSelectOne];
		
		[segmentedControl setSegmentStyle:NSSegmentStyleTexturedSquare];
        
        const NSSize groupSize = NSMakeSize(210.0, 25.0);
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
		
		[segmentedControl bind:@"selectedIndex" toObject:self withKeyPath:@"ShowTransactionsViewType" options:0];
        
        [segmentedControl release];
        return [groupItem autorelease];
    }
	else
    {
        [item release];
        return nil;
    }
	
    return [item autorelease];
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
	
	std::vector<Account>::iterator it = m_Document.AccountBegin();
	int nAccount = 0;
	
	for (; it != m_Document.AccountEnd(); ++it, nAccount++)
	{
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		
		NSString *sAccountKey = [NSString stringWithFormat:@"a%d", nAccount];

		[indexBar addItem:@"accounts" key:sAccountKey title:sName item:nAccount action:@selector(accountSelected:) target:self type:1 rename:@selector(accountRenamed:) renameTarget:self];
		
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
	m_pGraph = 0;
	
	m_SelectedTransaction = 0; 
	m_bEditing = false;
	
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
		
	m_pAccount = m_Document.getAccountPtr(nAccount);			
	m_SelectedTransaction = 0;
	
	[vTransactionsView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vTransactionsView];
	contentView = vTransactionsView;
	
	nViewType = 0;
	
	[transactionsPayee setStringValue:@""];
	[transactionsDescription setStringValue:@""];
	[transactionsCategory setStringValue:@""];
	[transactionsAmount setStringValue:@""];
	[transactionsType selectItemAtIndex:0];
	[transactionsCleared setState:NSOffState];
	
	[transactionsTableView deselectAll:self];
	
	[self buildTransactionsTree];
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
	
	m_pGraph = m_Document.getGraphPtr(nGraph);
	
	if (!m_pGraph)
		return;
	
	[vGraphView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vGraphView];
	contentView = vGraphView;
	
	nViewType = 4;
	
	// Update the list of Accounts
	
	[graphAccount removeAllItems];
	
	std::vector<Account>::iterator it = m_Document.AccountBegin();
	
	for (; it != m_Document.AccountEnd(); ++it)
	{
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		[graphAccount addItemWithTitle:sName];
		
		[sName release];
	}
	
	int nAccount = m_pGraph->getAccount();
	
	[graphAccount selectItemAtIndex:nAccount];
	
	NSDate *nsStartDate = convertToNSDate(const_cast<Date&>(m_pGraph->getStartDate1()));
	[graphStartDateCntrl setDateValue:nsStartDate];
	
	NSDate *nsEndDate = convertToNSDate(const_cast<Date&>(m_pGraph->getEndDate1()));
	[graphEndDateCntrl setDateValue:nsEndDate];
	
	GraphType eType = m_pGraph->getType();
	
	[graphType selectItemAtIndex:eType];
	
	if (m_pGraph->getIgnoreTransfers())
	{
		[graphIgnoreTransfers setState:NSOnState];
	}
	else
	{
		[graphIgnoreTransfers setState:NSOffState];
	}
	
	GraphItemsType eItemsType = m_pGraph->getItemsType();
	[graphItemTypes selectItemAtIndex:eItemsType];
	
	[m_aGraphItems removeAllObjects];
	
	std::set<std::string> &aItems = m_pGraph->getItems();
	
	for (std::set<std::string>::iterator it = aItems.begin(); it != aItems.end(); ++it)
	{
		std::string strItem = (*it);
		
		NSString *sItem = [[NSString alloc] initWithUTF8String:strItem.c_str()];
		
		[m_aGraphItems addObject:sItem];		
	}
	
	[graphItemsTableView reloadData];
	
	GraphDateType eDateType = m_pGraph->getDateType();
	
	int nDatePeriodSegment = 0;
	
	switch (eDateType)
	{
		case DateWeek:
			nDatePeriodSegment = 1;
			break;
		case DateMonth:
			nDatePeriodSegment = 2;
			break;
		case DateYear:
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
	
	GraphViewType eViewType = m_pGraph->getViewType();
	
	[graphViewType selectTabViewItemAtIndex:eViewType];
	
	// for some reason, the above sets/selects don't always update the controls properly
	[graphAccount setNeedsDisplay:YES];
	[graphStartDateCntrl setNeedsDisplay:YES];
	[graphEndDateCntrl setNeedsDisplay:YES];
	
	[self buildGraph:nAccount startDate:nsStartDate endDate:nsEndDate type:eType ignoreTransfers:m_pGraph->getIgnoreTransfers()];
}

// handle renames in the IndexBar

- (void)accountRenamed:(id)sender
{
	int nAccount = [sender getItemIndex];
	
	m_pAccount = m_Document.getAccountPtr(nAccount);
	
	NSString *title = [sender title];
	
	std::string strName = [title cStringUsingEncoding:NSUTF8StringEncoding];
	
	m_pAccount->setName(strName);
	
	m_UnsavedChanges = true;
	
	[indexBar reloadData];
	[indexBar setNeedsDisplay:YES];
}

- (void)graphRenamed:(id)sender
{
	int nGraph = [sender getItemIndex];
	
	m_pGraph = m_Document.getGraphPtr(nGraph);
	
	NSString *title = [sender title];
	
	std::string strName = [title cStringUsingEncoding:NSUTF8StringEncoding];
	
	m_pGraph->setName(strName);
	
	m_UnsavedChanges = true;
	
	[indexBar reloadData];
}

- (void)setWindowTitleWithDocName:(NSString*)path
{
	NSString *sWindowTitle;
	if (path)
	{
		sWindowTitle = [NSString stringWithFormat:@"Stash - %@", [[NSURL fileURLWithPath:path] lastPathComponent]];
	}
	else
	{
		sWindowTitle = @"Stash";
	}
	
	[[self window] setTitle:sWindowTitle];	
}

- (void)buildTransactionsTree
{
	[m_aTransactionItems removeAllObjects];
	
	fixed localBalance = 0.0;
	
	if (!m_pAccount)
	{
		[transactionsTableView reloadData];
		return;
	}
	
	if (m_pAccount->getTransactionCount() == 0)
	{
		[transactionsTableView reloadData];
		return;
	}
	
	std::vector<Transaction>::const_iterator it = m_pAccount->begin();
	int nTransaction = 0;
	m_nTransactionOffset = 0;
	
	if (ShowTransactionsViewType == RECENT)
	{
		int nRecentDuration = [[NSUserDefaults standardUserDefaults] integerForKey:@"TransactionsRecentDuration"];
		
		if (nRecentDuration <= 0)
			nRecentDuration = 30;
		
		Date dateNow;
		dateNow.Now();
		
		dateNow.DecrementDays(nRecentDuration);
		
		std::vector<Transaction>::const_iterator itTemp = m_pAccount->begin();
		
		for (; itTemp != m_pAccount->end(); ++itTemp)
		{
			if ((*itTemp).getDate1() >= dateNow)
				break;
			
			localBalance += (*itTemp).getAmount();
			nTransaction++;
		}
		
		it = itTemp;
		
		m_nTransactionOffset = nTransaction;
	}
	else if (ShowTransactionsViewType == ALL_THIS_YEAR)
	{
		Date dateNow;
		dateNow.Now();
		
		Date dateComp(1, 1, dateNow.getYear());
		
		std::vector<Transaction>::const_iterator itTemp = m_pAccount->begin();
		
		for (; itTemp != m_pAccount->end(); ++itTemp)
		{
			if ((*itTemp).getDate1() >= dateComp)
				break;
			
			localBalance += (*itTemp).getAmount();
			nTransaction++;
		}
		
		it = itTemp;
		
		m_nTransactionOffset = nTransaction;
	}
	
	m_aBalance.clear();
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	BOOL bShowNegAmountsInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegAmountsRed"];
	BOOL bShowNegBalancesInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegBalancesRed"];
	
	for (; it != m_pAccount->end(); ++it, nTransaction++)
	{
		TransactionItem *newTransaction = [[TransactionItem alloc] init];
		
		std::string strTPayee = it->getPayee();
		NSString *sTPayee = [[NSString alloc] initWithUTF8String:strTPayee.c_str()];
		
		std::string strTDescription = it->getDescription();
		NSString *sTDescription = [[NSString alloc] initWithUTF8String:strTDescription.c_str()];
		
		std::string strTCategory = it->getCategory();
		NSString *sTCategory = [[NSString alloc] initWithUTF8String:strTCategory.c_str()];
		
		NSString *sTType = [self transactionTypeToString:it->getType()];
		
		NSNumber *nTAmount = [NSNumber numberWithDouble:it->getAmount().ToDouble()];
		
		NSString *sTAmount = [[numberFormatter stringFromNumber:nTAmount] retain];
		
		NSDate *date = convertToNSDate(const_cast<Date&>(it->getDate1()));
		NSString *sTDate = [[dateFormatter stringFromDate:date] retain];
		
		localBalance += it->getAmount();
		
		m_aBalance.push_back(localBalance);
		
		NSNumber *nTBalance = [NSNumber numberWithDouble:localBalance.ToDouble()];
		NSString *sTBalance = [[numberFormatter stringFromNumber:nTBalance] retain];
		
		[newTransaction setTransaction:nTransaction];
		
		int recon = it->isCleared();
		
		BOOL bIsAmountNeg = NO;
		BOOL bIsBalanceNeg = NO;
		
		if (bShowNegAmountsInRed && !it->getAmount().IsPositive())
			bIsAmountNeg = YES;
		
		if (bShowNegBalancesInRed && !localBalance.IsPositive())
			bIsBalanceNeg = YES;
		
		[newTransaction setIntValue:recon forKey:@"Cleared"];
		[newTransaction setValue:sTDate forKey:@"Date"];
		[newTransaction setValue:sTPayee forKey:@"Payee"];
		[newTransaction setValue:sTDescription forKey:@"Description"];
		[newTransaction setValue:sTCategory forKey:@"Category"];
		[newTransaction setValue:sTType forKey:@"Type"];
		[newTransaction setValue:sTAmount forKey:@"Amount"];
		[newTransaction setValue:sTBalance forKey:@"Balance"];
		
		[newTransaction setBoolValue:bIsAmountNeg forKey:@"AmoNeg"];
		[newTransaction setBoolValue:bIsBalanceNeg forKey:@"BalNeg"];
		
		[newTransaction setIntValue:nTransaction forKey:@"Transaction"];
		
		if (it->isSplit())
		{
			int nSplits = it->getSplitCount();
			
			fixed splitValue = it->getAmount();
			
			for (int i = 0; i < nSplits; i++)
			{
				const SplitTransaction &split = it->getSplit(i);
				
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				std::string strSPayee = split.getPayee();
				NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
				
				std::string strSDescription = split.getDescription();
				NSString *sSDescription = [[NSString alloc] initWithUTF8String:strSDescription.c_str()];
				
				std::string strSCategory = split.getCategory();
				NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
				
				NSNumber *nSAmount = [NSNumber numberWithDouble:split.getAmount().ToDouble()];
				NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
				
				bIsAmountNeg = NO;
			
				if (bShowNegAmountsInRed && !split.getAmount().IsPositive())
					bIsAmountNeg = YES;
				
				[newSplit setValue:sSPayee forKey:@"Payee"];
				[newSplit setValue:sSDescription forKey:@"Description"];
				[newSplit setValue:sSCategory forKey:@"Category"];
				[newSplit setValue:sSAmount forKey:@"Amount"];
				
				[newSplit setBoolValue:bIsAmountNeg forKey:@"AmoNeg"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:i];
				[newSplit setIntValue:i forKey:@"Split"];
				
				splitValue -= split.getAmount();
				
				[newTransaction addChild:newSplit];
				
				[sSPayee release];
				[sSDescription release];
				[sSCategory release];
				[sSAmount release];
			}
			
			// add remainder as a temp editable row
			
			if (!splitValue.IsZero())
			{
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				NSNumber *nSAmount = [NSNumber numberWithDouble:splitValue.ToDouble()];
				NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
				
				[newSplit setValue:@"Split Value" forKey:@"Description"];
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:sSAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:-2];
				[newSplit setIntValue:-2 forKey:@"Split"];
				
				[newTransaction addChild:newSplit];
				[sSAmount release];
			}
		}		
		
		[m_aTransactionItems addObject:newTransaction];
		[sTPayee release];
		[sTDescription release];
		[sTCategory release];
		[sTAmount release];
		[sTDate release];
		[sTBalance release];
	}
	
	[dateFormatter release];
	[numberFormatter release];
	
	[transactionsTableView reloadData];
	
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsScrollToLatest"] == YES)
	{
		int latest = [transactionsTableView numberOfRows];
		[transactionsTableView scrollRowToVisible:latest - 1];
	}
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
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	std::vector<ScheduledTransaction>::iterator it = m_Document.SchedTransBegin();
	
	int schedTransIndex = 0;
	
	for (; it != m_Document.SchedTransEnd(); ++it, schedTransIndex++)
	{
		NSMutableDictionary *item = [[NSMutableDictionary alloc] init];
		
		std::string strSPayee = it->getPayee();
		NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
		
		std::string strSCategory = it->getCategory();
		NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
		
		NSNumber *nSAmount = [NSNumber numberWithDouble:it->getAmount().ToDouble()];
		
		NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
		
		NSDate *date = convertToNSDate(const_cast<Date&>(it->getNextDate2()));
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
		
		[m_aScheduledTransactions addObject:item];
	}
	
	[dateFormatter release];
	[numberFormatter release];
	
	[scheduledTransactionsTableView reloadData];
}

- (void)buildGraph:(int)account startDate:(NSDate*)startDate endDate:(NSDate*)endDate type:(GraphType)type ignoreTransfers:(bool)ignoreTransfers
{
	if (!m_pGraph)
		return;
		
	if (account < 0)
		return;
	
	Account *pAccount = m_Document.getAccountPtr(account);
			
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
	
	std::vector<PieChartItem> aPieChartItems;
	
	fixed overallTotal = 0.0;
	
	int pieSmallerThanValue = -1;
	std::string pieGroupSmallerName = "";
	
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"PieChartGroupSmallerItems"] == YES)
	{
		pieSmallerThanValue = [[NSUserDefaults standardUserDefaults] integerForKey:@"PieChartGroupSmallerItemsSize"];
		NSString *sGroupOtherName = [[NSUserDefaults standardUserDefaults] valueForKey:@"PieChartGroupSmallerItemsName"];
		
		pieGroupSmallerName = [sGroupOtherName cStringUsingEncoding:NSUTF8StringEncoding]; 
	}
	
	PieChartSort ePieChartSort = static_cast<PieChartSort>([[NSUserDefaults standardUserDefaults] integerForKey:@"PieChartSortType"]);
	
	PieChartCriteria pieCriteria(pAccount, aPieChartItems, mainStartDate, mainEndDate, overallTotal, ignoreTransfers, pieSmallerThanValue, pieGroupSmallerName, ePieChartSort);
	
	//
	
	int nItemsType = [graphItemTypes indexOfSelectedItem];
	GraphItemsType eItemsType = static_cast<GraphItemsType>(nItemsType);
	
	pieCriteria.m_itemsType = eItemsType;
	
	std::set<std::string> &aItems = pieCriteria.m_aItems;
	
	for (NSString *sItem in m_aGraphItems)
	{
		std::string strItem = [sItem cStringUsingEncoding:NSUTF8StringEncoding];
		
		aItems.insert(strItem);
	}
		
	if (type == ExpenseCategories)
		buildPieChartItems(m_pGraph, pieCriteria, true, true);
	else if (type == ExpensePayees)
		buildPieChartItems(m_pGraph, pieCriteria, true, false);
	else if (type == DepositCategories)
		buildPieChartItems(m_pGraph, pieCriteria, false, true);
	else if (type == DepositPayees)
		buildPieChartItems(m_pGraph, pieCriteria, false, false);
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLenient:YES];
	
	NSMutableArray *aPieItems = [[NSMutableArray alloc] init];
	
	std::vector<PieChartItem>::iterator itPie = aPieChartItems.begin();
	
	double startAngle = 0.0;
	
	for (; itPie != aPieChartItems.end(); ++itPie)
	{
		NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
		
		std::string strTitle = (*itPie).getTitle();
		fixed amount = (*itPie).getAmount();
		double angle = (*itPie).getAngle();
		
		NSString *sTitle = [[NSString alloc] initWithUTF8String:strTitle.c_str()];
		
		NSNumber *nSAmount = [NSNumber numberWithDouble:amount.ToDouble()];
		NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
		
		[dict setValue:[NSNumber numberWithDouble:angle] forKey:@"angle"];
		
		[dict setValue:[NSNumber numberWithDouble:startAngle] forKey:@"startangle"];
		
		startAngle += angle;
		
		[dict setValue:[NSNumber numberWithDouble:startAngle] forKey:@"endangle"];		 
		
		[dict setValue:sTitle forKey:@"title"];
		[dict setValue:sSAmount forKey:@"amount"];
		
		[dict setValue:nSAmount forKey:@"numamount"];
				
		[aPieItems addObject:dict];		
	}
	
	NSNumber *nTotal = [NSNumber numberWithDouble:overallTotal.ToDouble()];
	NSString *sTotal = [[numberFormatter stringFromNumber:nTotal] retain];
	
	[numberFormatter release];
	
	[pieChartView setTotal:sTotal];
	[pieChartView setData:aPieItems];
	
	// Area Chart Items
	
	std::vector<AreaChartItem> aAreaChartItems;
	std::vector<MonthYear> aDateItems;
	
	fixed overallMax = 0.0;
	
	int areaSmallerThanValue = -1;
	std::string areaGroupSmallerName = "";
	
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"AreaChartGroupSmallerItems"] == YES)
	{
		areaSmallerThanValue = [[NSUserDefaults standardUserDefaults] integerForKey:@"AreaChartGroupSmallerItemsSize"];
		NSString *sGroupOtherName = [[NSUserDefaults standardUserDefaults] valueForKey:@"AreaChartGroupSmallerItemsName"];
		
		areaGroupSmallerName = [sGroupOtherName cStringUsingEncoding:NSUTF8StringEncoding]; 
	}
	
	AreaChartCriteria areaCriteria(pAccount, aAreaChartItems, aDateItems, mainStartDate, mainEndDate, overallMax, ignoreTransfers,
								   areaSmallerThanValue, areaGroupSmallerName);
	
	areaCriteria.m_itemsType = eItemsType;
	
	areaCriteria.m_aItems = pieCriteria.m_aItems;
	
	if (type == ExpenseCategories)
		buildAreaChartItems(m_pGraph, areaCriteria, true, true);
	else if (type == ExpensePayees)
		buildAreaChartItems(m_pGraph, areaCriteria, true, false);
	else if (type == DepositCategories)
		buildAreaChartItems(m_pGraph, areaCriteria, false, true);
	else if (type == DepositPayees)
		buildAreaChartItems(m_pGraph, areaCriteria, false, false);
			
	NSMutableArray *aAreaItems = [[NSMutableArray alloc] init];
	
	std::vector<AreaChartItem>::iterator itArea = aAreaChartItems.begin();
	
	for (; itArea != aAreaChartItems.end(); ++itArea)
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
	
	std::vector<MonthYear>::iterator itDate = aDateItems.begin();
	
	int nLongestDateLength = 0;
	int nLongestDateIndex = -1;
	
	int nIndex = 0;
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateFormat:@"MMM\nyyyy"];
	
	for (; itDate != aDateItems.end(); ++itDate, nIndex++)
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
		[areaChartView setMaxValue:overallMax.ToDouble()];
		[areaChartView setData:aAreaItems];
	}
	
	// Overview chart
	
	overallMax = 0.0;
	
	OverviewChartCriteria overviewCriteria(pAccount, mainStartDate, mainEndDate, overallMax, ignoreTransfers);
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

- (void)refreshLibraryItems
{
	[transactionsPayee removeAllItems];
	[scheduledPayee removeAllItems];
	
	std::set<std::string>::iterator it = m_Document.PayeeBegin();
	
	for (; it != m_Document.PayeeEnd(); ++it)
	{
		NSString *sPayee = [[NSString alloc] initWithUTF8String:(*it).c_str()];
		
		[transactionsPayee addItemWithObjectValue:sPayee];
		[scheduledPayee addItemWithObjectValue:sPayee];
	}
	
	[transactionsCategory removeAllItems];
	[scheduledCategory removeAllItems];
	
	std::set<std::string>::iterator itCat = m_Document.CategoryBegin();
	
	for (; itCat != m_Document.CategoryEnd(); ++itCat)
	{
		NSString *sCategory = [[NSString alloc] initWithUTF8String:(*itCat).c_str()];
		
		[transactionsCategory addItemWithObjectValue:sCategory];
		[scheduledCategory addItemWithObjectValue:sCategory];
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
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLenient:YES];
	
	if ([[numberFormatter currencyCode] isEqualToString:@"USD"])
	{
		sStartingBalance = [self convertUSNegAmount:sStartingBalance];
	}
	
	NSNumber *nStartingBalance = [numberFormatter numberFromString:sStartingBalance];
	
	fixed startingBalance = [nStartingBalance doubleValue];
	
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
	
	int nAccountNum = m_Document.addAccount(newAccount);
	
	NSString *sAccountKey = [NSString stringWithFormat:@"a@s", nAccountNum];
	
	[indexBar addItem:@"accounts" key:sAccountKey title:sAccountName item:nAccountNum action:@selector(accountSelected:) target:self type:1 rename:@selector(accountRenamed:) renameTarget:self];
	
	[addAccountController release];
	[numberFormatter release];
	
	m_UnsavedChanges = true;
	
	[indexBar reloadData];
	
	if (nAccountNum == 0) // if first account added, select it
	{
		[indexBar selectItem:sAccountKey];
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
	
	m_UnsavedChanges = true;
	
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
		
		m_UnsavedChanges = true;
		
		[self buildIndexTree];
	}
}

- (IBAction)AddTransaction:(id)sender
{
	if (!m_pAccount)
	{
		NSRunAlertPanel(NSLocalizedString(@"No Active Account", "No Active Account Warning Title"),
						NSLocalizedString(@"You must have an active account in order to add a new transaction.", "No Active Account Warning Message"),
										  NSLocalizedString(@"OK", "OK Button"), nil, nil);
		return;
	}
	
	NSDate *ndate1 = [transactionsDateCntl dateValue];
	NSCalendarDate *CalDate = [ndate1 dateWithCalendarFormat:0 timeZone:0];
	
	int nYear = [CalDate yearOfCommonEra];
	int nMonth = [CalDate monthOfYear];
	int nDay = [CalDate dayOfMonth];
	
	Date date1(nDay, nMonth, nYear);
	
	Transaction newTransaction("", "", "", 0.0, date1);
	
	int nTransaction = m_pAccount->addTransaction(newTransaction);
	
	[transactionsPayee setStringValue:@""];
	[transactionsDescription setStringValue:@""];
	[transactionsCategory setStringValue:@""];
	[transactionsAmount setStringValue:@""];
	[transactionsType selectItemAtIndex:0];
	[transactionsCleared setState:NSOffState];
	
	TransactionItem *newIndex = [self createTransactionItem:newTransaction index:nTransaction];
	
	[m_aTransactionItems addObject:newIndex];
	[newIndex release];
	
	[transactionsTableView reloadData];
	
	NSInteger row = [transactionsTableView rowForItem:newIndex];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	[transactionsTableView scrollRowToVisible:row];
	
	[window makeFirstResponder:transactionsPayee];
	
	m_UnsavedChanges = true;
}

- (IBAction)DeleteTransaction:(id)sender
{
	NSIndexSet *rows = [transactionsTableView selectedRowIndexes];
	
	int nCount = [rows count];
	
	if (m_pAccount && nCount > 0)
	{
		NSInteger row = [rows lastIndex];
		
		int nRow = row;
		
		int nSplit = -1;
		int nTransaction = -1;
		
		while (row != NSNotFound)
		{
			nRow = row;
			
			TransactionItem *item = [transactionsTableView itemAtRow:row];
			
			nSplit = [item intKeyValue:@"Split"];
			nTransaction = [item intKeyValue:@"Transaction"];
			
			if (nSplit == -1)
			{
				m_pAccount->deleteTransaction(nTransaction);
				[m_aTransactionItems removeObjectAtIndex:nTransaction - m_nTransactionOffset];
			}
			else if (nSplit != -2)
			{
				TransactionItem *transactionItem = [m_aTransactionItems objectAtIndex:nTransaction - m_nTransactionOffset];
				Transaction &trans = m_pAccount->getTransaction(nTransaction);
				
				trans.deleteSplit(nSplit);
				[transactionItem deleteChild:nSplit];
			}
			
			if (nSplit == -1 && nTransaction >= 0)
			{
				[self updateBalancesFromTransactionIndex:nTransaction - m_nTransactionOffset];
				
				[self updateTransactionsFromTransactionIndex:nTransaction - m_nTransactionOffset];
			}
			
			row = [rows indexLessThanIndex:row];	
		}		
		
		[transactionsTableView reloadData];
		
		[transactionsTableView deselectAll:self];
		
		m_UnsavedChanges = true;
	}	
}

- (IBAction)SplitTransaction:(id)sender
{
	NSInteger row = [transactionsTableView selectedRow];
	
	if (row == NSNotFound)
		return;
	
	TransactionItem *item = [transactionsTableView itemAtRow:row];
	
	int nTransaction = [item intKeyValue:@"Transaction"];
	Transaction &trans = m_pAccount->getTransaction(nTransaction);
	
	fixed splitValue = trans.getAmount();
	TransactionItem *newSplit = [[TransactionItem alloc] init];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	NSNumber *nAmount = [NSNumber numberWithDouble:splitValue.ToDouble()];
	NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
	
	[newSplit setValue:@"Split Value" forKey:@"Description"];
	[newSplit setValue:@"Split Value" forKey:@"Payee"];
	[newSplit setValue:sAmount forKey:@"Amount"];
	
	[newSplit setTransaction:nTransaction];
	[newSplit setIntValue:nTransaction forKey:@"Transaction"];
	
	[newSplit setSplitTransaction:-2];
	[newSplit setIntValue:-2 forKey:@"Split"];
	
	[item addChild:newSplit];
	
	[transactionsTableView reloadData];
	
	[transactionsTableView expandItem:item];
	
	row = [transactionsTableView rowForItem:newSplit];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	[transactionsTableView scrollRowToVisible:row];
	
	[window makeFirstResponder:transactionsPayee];
	
	[numberFormatter release];
	
	m_UnsavedChanges = true;
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
	
	if (!makeTransferController)
		makeTransferController = [[MakeTransferController alloc] initWithAccounts:aAccounts categories:aCategories];
	
	[makeTransferController makeTransfer:window initialAccount:0];	
}

- (void)makeTransferItem:(MakeTransferController *)makeTransferCont
{
	int nFromAccount = [makeTransferCont fromAccount];
	int nToAccount = [makeTransferCont toAccount];
	
	Account *pFromAccount = NULL;
	pFromAccount = m_Document.getAccountPtr(nFromAccount);
	Account *pToAccount = NULL;
	pToAccount = m_Document.getAccountPtr(nToAccount);
	
	if (!pFromAccount || !pToAccount)
	{
		
		return;
	}
	
	NSString *sAmount = [makeTransferCont amount];
	NSString *sCategory = [makeTransferCont category];
	NSString *sDescription = [makeTransferCont description];
	NSDate *dtDate = [makeTransferCont date];
	BOOL bMakeCleared = [makeTransferCont makeCleared];
		
	std::string strCategory = [sCategory cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strDescription = [sDescription cStringUsingEncoding:NSUTF8StringEncoding];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLenient:YES];
	
	if ([[numberFormatter currencyCode] isEqualToString:@"USD"])
	{
		sAmount = [self convertUSNegAmount:sAmount];
	}
	
	NSNumber *nAmount = [numberFormatter numberFromString:sAmount];
	
	fixed amount = [nAmount doubleValue];
	
	NSCalendarDate *CalDate = [dtDate dateWithCalendarFormat:0 timeZone:0];
	
	int nYear = [CalDate yearOfCommonEra];
	int nMonth = [CalDate monthOfYear];
	int nDay = [CalDate dayOfMonth];
	
	Date date1(nDay, nMonth, nYear);
	
	std::string strFromAccountName = pFromAccount->getName();
	std::string strToAccountName = pToAccount->getName();
	
	amount.setNegative();
	
	Transaction fromTransaction(strDescription, strToAccountName, strCategory, amount, date1);
	fromTransaction.setType(Transfer);
	if (bMakeCleared)
	{
		fromTransaction.setCleared(true);
	}
	int nFromTransaction = pFromAccount->addTransaction(fromTransaction);
	
	amount.setPositive();
	
	Transaction toTransaction(strDescription, strFromAccountName, strCategory, amount, date1);
	toTransaction.setType(Transfer);
	if (bMakeCleared)
	{
		toTransaction.setCleared(true);
	}
	int nToTransaction = pToAccount->addTransaction(toTransaction);
	
//	[makeTransferController release];
	[numberFormatter release];
	
	if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
	{
		m_Document.addCategory(strCategory);
		[transactionsCategory addItemWithObjectValue:sCategory];
	}
	
	m_UnsavedChanges = true;
	
	if (m_pAccount && (m_pAccount == pFromAccount || m_pAccount == pToAccount))
	{
		TransactionItem *newTransaction = NULL;
		
		if (m_pAccount == pFromAccount)
		{
			newTransaction = [self createTransactionItem:fromTransaction index:nFromTransaction];			
		}
		else if (m_pAccount == pToAccount)
		{
			newTransaction = [self createTransactionItem:toTransaction index:nToTransaction];			
		}
		
		[m_aTransactionItems addObject:newTransaction];
		[newTransaction release];
		
		[transactionsTableView reloadData];
		
		NSInteger row = [transactionsTableView rowForItem:newTransaction];
		
		[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
		[transactionsTableView scrollRowToVisible:row];
	}
}

- (IBAction)MoveUp:(id)sender
{
	if (!m_pAccount)
		return;
	
	NSIndexSet *rows = [transactionsTableView selectedRowIndexes];
	
	if ([rows count] == 1)
	{
		NSInteger row = [rows lastIndex];
		
		TransactionItem *thisItem = [transactionsTableView itemAtRow:row];
		
		int nExactItemIndex = [thisItem transaction];
		
		int nSplit = [thisItem splitTransaction];
		
		if (nSplit >= 0)
		{
			return;
		}
		
		// Swap them over
		[self SwapTransactions:nExactItemIndex to:nExactItemIndex - 1];
	}
}

- (IBAction)MoveDown:(id)sender
{
	if (!m_pAccount)
		return;
	
	NSIndexSet *rows = [transactionsTableView selectedRowIndexes];
	
	if ([rows count] == 1)
	{
		NSInteger row = [rows lastIndex];
		
		TransactionItem *thisItem = [transactionsTableView itemAtRow:row];
		
		int nExactItemIndex = [thisItem transaction];
		
		int nSplit = [thisItem splitTransaction];
		
		if (nSplit >= 0)
		{
			return;
		}
		
		// Swap them over
		[self SwapTransactions:nExactItemIndex to:nExactItemIndex + 1];
	}
}

- (void)SwapTransactions:(int)from to:(int)to
{
	if (from < 0 || to < 0)
		return;
	
	int top = m_pAccount->getTransactionCount();
	
	if (from >= top || to >= top)
		return;
	
	int nItemFrom = from - m_nTransactionOffset;
	int nItemTo = to - m_nTransactionOffset;
	
	m_pAccount->swapTransactions(from, to);
	[m_aTransactionItems exchangeObjectAtIndex:nItemFrom withObjectAtIndex:nItemTo];
	
	// TransactionItem values are still pointing in the wrong place, so fix them....
	
	TransactionItem *fromItem = [m_aTransactionItems objectAtIndex:nItemFrom];
	TransactionItem *toItem = [m_aTransactionItems objectAtIndex:nItemTo];
	
	[fromItem setTransaction:from];
	[fromItem setIntValue:from forKey:@"Transaction"];
	[fromItem setChildrenTransactionIndex:from];
	
	[toItem setTransaction:to];
	[toItem setIntValue:to forKey:@"Transaction"];
	[toItem setChildrenTransactionIndex:to];
	
	if (from < to)
	{
		[self updateBalancesFromTransactionIndex:nItemFrom];
	}
	else
	{
		[self updateBalancesFromTransactionIndex:nItemTo];
	}
	
	[transactionsTableView reloadData];
	
	int nNewRow = [transactionsTableView rowForItem:toItem];
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:nNewRow] byExtendingSelection:NO];
	[transactionsTableView scrollRowToVisible:nNewRow];
	
	m_UnsavedChanges = true;
}

- (void)RefreshView:(id)sender
{
	[self buildTransactionsTree];
	
	[transactionsTableView reloadData];	
}

- (TransactionItem*)createTransactionItem:(Transaction&)transaction index:(int)index
{
	TransactionItem *newItem = [[TransactionItem alloc] init];
	
	std::string strPayee = transaction.getPayee();
	NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
	
	std::string strDescription = transaction.getDescription();
	NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
	
	std::string strCategory = transaction.getCategory();
	NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
	
	NSString *sType = [self transactionTypeToString:transaction.getType()];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	NSNumber *nAmount = [NSNumber numberWithDouble:transaction.getAmount().ToDouble()];
	NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
	
	NSDate *transDate = convertToNSDate(const_cast<Date&>(transaction.getDate1()));
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	NSString *sDate = [[dateFormatter stringFromDate:transDate] retain];
	
	[dateFormatter release];
	
	fixed localBalance;
	
	if (!m_aBalance.empty())
	{
		localBalance = m_aBalance.back();
	}
	else
	{
		localBalance = m_pAccount->getBalance(true);
	}
	
	localBalance += transaction.getAmount();
	
	NSNumber *nBalance = [NSNumber numberWithDouble:localBalance.ToDouble()];
	NSString *sBalance = [[numberFormatter stringFromNumber:nBalance] retain];
	
	[numberFormatter release];
	
	[newItem setTransaction:index];
	
	int cleared = transaction.isCleared();
	
	[newItem setIntValue:cleared forKey:@"Cleared"];
	[newItem setValue:sDate forKey:@"Date"];
	[newItem setValue:sPayee forKey:@"Payee"];
	[newItem setValue:sDescription forKey:@"Description"];
	[newItem setValue:sCategory forKey:@"Category"];
	[newItem setValue:sType forKey:@"Type"];
	[newItem setValue:sAmount forKey:@"Amount"];
	[newItem setValue:sBalance forKey:@"Balance"];
	
	[newItem setIntValue:index forKey:@"Transaction"];
	
	return newItem;
}

- (void)TransactionSelectionDidChange:(NSNotification *)notification
{
	if (!m_pAccount)
		return;
	
	NSIndexSet *rows = [transactionsTableView selectedRowIndexes];
	
	if ([rows count] == 1)
	{
		[deleteTransaction setEnabled:YES];
		
		NSInteger row = [rows lastIndex];
		
		TransactionItem *item = [[transactionsTableView itemAtRow:row] retain];
		
		int nTrans = [item transaction];
		int nSplit = [item splitTransaction];
		
		Transaction *trans = NULL;
		SplitTransaction *split = NULL;
		
		if (nTrans >= 0)
			trans = &m_pAccount->getTransaction(nTrans);
		
		if (nSplit >= 0)
			split = &trans->getSplit(nSplit);
		
		m_SelectedTransaction = item;
		
		NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
		[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
		[numberFormatter setLenient:YES];
		
		if (trans && !split && nSplit != -2) // A normal transaction
		{
			[splitTransaction setEnabled:YES];
			[moveUp setEnabled:YES];
			[moveDown setEnabled:YES];
			
			m_bEditing = true;			
			
			std::string strPayee = trans->getPayee();
			NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
			
			std::string strDescription = trans->getDescription();
			NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
			
			std::string strCategory = trans->getCategory();
			NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
			
			NSNumber *nAmount = [NSNumber numberWithDouble:trans->getAmount().ToDouble()];
			NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
			
			TransactionType eType = trans->getType();
			
			Date date1 = trans->getDate();
			NSDate *datetemp = convertToNSDate(date1);
			
			[transactionsCleared setEnabled:YES];
			[transactionsType setEnabled:YES];
			[transactionsDateCntl setEnabled:YES];
			
			bool bCleared = trans->isCleared();
			
			if (bCleared)
			{
				[transactionsCleared setState:NSOnState];
			}
			else
			{
				[transactionsCleared setState:NSOffState];
			}
			
			[transactionsPayee setStringValue:sPayee];
			[transactionsDescription setStringValue:sDescription];
			[transactionsCategory setStringValue:sCategory];
			[transactionsAmount setStringValue:sAmount];
			[transactionsDateCntl setDateValue:datetemp];
			[transactionsType selectItemAtIndex:eType];
			
			[sPayee release];
			[sDescription release];
			[sCategory release];
			[sAmount release];
		}
		else if (trans && split && nSplit != -2)
		{
			[splitTransaction setEnabled:NO];
			[moveUp setEnabled:NO];
			[moveDown setEnabled:NO];
			
			m_bEditing = true;
			
			std::string strPayee = split->getPayee();
			NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
			
			std::string strDescription = split->getDescription();
			NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
			
			std::string strCategory = split->getCategory();
			NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
			
			NSNumber *nAmount = [NSNumber numberWithDouble:split->getAmount().ToDouble()];
			NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
			
			[transactionsPayee setStringValue:sPayee];
			[transactionsDescription setStringValue:sDescription];
			[transactionsCategory setStringValue:sCategory];
			[transactionsAmount setStringValue:sAmount];
			[transactionsType selectItemAtIndex:0];
			
			[transactionsCleared setEnabled:NO];
			[transactionsType setEnabled:NO];
			[transactionsDateCntl setEnabled:NO];
			
			[sPayee release];
			[sDescription release];
			[sCategory release];
			[sAmount release];
		}
		else // Dummy Split
		{
			[splitTransaction setEnabled:NO];
			[moveUp setEnabled:NO];
			[moveDown setEnabled:NO];
			
			m_bEditing = true;
			
			NSString *sPayee = [item keyValue:@"Payee"];
			NSString *sDescription = [item keyValue:@"Description"];
			NSString *sAmount = [item keyValue:@"Amount"];
			
			[transactionsPayee setStringValue:sPayee];
			[transactionsDescription setStringValue:sDescription];
			[transactionsCategory setStringValue:@""];
			[transactionsAmount setStringValue:sAmount];
			[transactionsType selectItemAtIndex:0];
			
			[transactionsCleared setEnabled:NO];
			[transactionsType setEnabled:NO];
			[transactionsDateCntl setEnabled:NO];
		}
		
		[numberFormatter release];
	}
	else
	{
		if ([rows count] > 1)
		{
			[deleteTransaction setEnabled:YES];
		}
		else
		{
			[deleteTransaction setEnabled:NO];
		}
		
		[splitTransaction setEnabled:NO];
		[moveUp setEnabled:NO];
		[moveDown setEnabled:NO];
		
		[transactionsPayee setStringValue:@""];
		[transactionsDescription setStringValue:@""];
		[transactionsCategory setStringValue:@""];
		[transactionsType selectItemAtIndex:0];
		[transactionsAmount setStringValue:@""];
		[transactionsCleared setState:NSOffState];		
		
		m_SelectedTransaction = 0; 
		m_bEditing = false;		
	}
}

- (void)updateTransaction:(id)sender
{
	if (!m_bEditing || !m_SelectedTransaction)
		return;
	
	if ([[transactionsAmount stringValue] length] == 0)
	{
		[window makeFirstResponder:transactionsAmount];
		NSBeep();
		
		return;
	}
	
	NSDate *ndate1 = [transactionsDateCntl dateValue];
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
	
	std::string strPayee = [[transactionsPayee stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strDesc = [[transactionsDescription stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
	std::string strCategory = [[transactionsCategory stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLenient:YES];
	
	NSString *sTransactionAmount = [transactionsAmount stringValue];
	if ([[numberFormatter currencyCode] isEqualToString:@"USD"])
	{
		sTransactionAmount = [self convertUSNegAmount:sTransactionAmount];
	}
	
	NSNumber *nAmount = [numberFormatter numberFromString:sTransactionAmount];	
	fixed fAmount = [nAmount doubleValue];

	// reformat the number again, in case an abbreviation was used
	NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
	
	int nType = [transactionsType indexOfSelectedItem];
	TransactionType eType = static_cast<TransactionType>(nType);
	
	NSString *sType = [self transactionTypeToString:eType];
	
	bool bCleared = false;
	
	if ([transactionsCleared state] == NSOnState)
		bCleared = true;
	
	int nTrans = [m_SelectedTransaction transaction];
	int nSplit = [m_SelectedTransaction splitTransaction];
	
	Transaction *trans = NULL;
	SplitTransaction *split = NULL;
	
	if (nTrans >= 0)
		trans = &m_pAccount->getTransaction(nTrans);
	
	if (nSplit >= 0)
		split = &trans->getSplit(nSplit);
	
	BOOL bIsAmountNeg = NO;
	
	BOOL bShowNegAmountsInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegAmountsRed"];
	
	if (trans && !split && nSplit != -2)
	{
		trans->setDate(date1);
		trans->setPayee(strPayee);
		trans->setDescription(strDesc);
		trans->setCategory(strCategory);
		trans->setType(eType);
		trans->setCleared(bCleared);
		
		fixed oldAmount = trans->getAmount();
		
		if (oldAmount != fAmount)
		{
			trans->setAmount(fAmount);
			[self updateBalancesFromTransactionIndex:nTrans - m_nTransactionOffset];
		}
		
		if (bShowNegAmountsInRed && !fAmount.IsPositive())
			bIsAmountNeg = YES;
		
		[m_SelectedTransaction setValue:sDate forKey:@"Date"];
		[m_SelectedTransaction setValue:[transactionsPayee stringValue] forKey:@"Payee"];
		[m_SelectedTransaction setValue:[transactionsDescription stringValue] forKey:@"Description"];
		[m_SelectedTransaction setValue:[transactionsCategory stringValue] forKey:@"Category"];
		[m_SelectedTransaction setValue:sType forKey:@"Type"];
		[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
		[m_SelectedTransaction setIntValue:bCleared forKey:@"Cleared"];
		[m_SelectedTransaction setBoolValue:bIsAmountNeg forKey:@"AmoNeg"];
		
		if (!strPayee.empty() && !m_Document.doesPayeeExist(strPayee))
		{
			m_Document.addPayee(strPayee);
			[transactionsPayee addItemWithObjectValue:[transactionsPayee stringValue]];
		}
		
		if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
		{
			m_Document.addCategory(strCategory);
			[transactionsCategory addItemWithObjectValue:[transactionsCategory stringValue]];
		}
		
		[transactionsTableView reloadData];
	}
	else
	{
		if (nSplit != -2)
		{
			split->setPayee(strPayee);
			split->setDescription(strDesc);
			split->setCategory(strCategory);
			split->setAmount(fAmount);
			
			if (bShowNegAmountsInRed && !fAmount.IsPositive())
				bIsAmountNeg = YES;
			
			[m_SelectedTransaction setValue:[transactionsPayee stringValue] forKey:@"Payee"];
			[m_SelectedTransaction setValue:[transactionsDescription stringValue] forKey:@"Description"];
			[m_SelectedTransaction setValue:[transactionsCategory stringValue] forKey:@"Category"];
			[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
			[m_SelectedTransaction setIntValue:bCleared forKey:@"Cleared"];
			[m_SelectedTransaction setBoolValue:bIsAmountNeg forKey:@"AmoNeg"];
			
			if (!strPayee.empty() && !m_Document.doesPayeeExist(strPayee))
			{
				m_Document.addPayee(strPayee);
				[transactionsPayee addItemWithObjectValue:[transactionsPayee stringValue]];
			}
			
			if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
			{
				m_Document.addCategory(strCategory);
				[transactionsCategory addItemWithObjectValue:[transactionsCategory stringValue]];
			}
			
			[transactionsTableView reloadData];
		}
		else if (nSplit == -2) // Dummy value, so convert to a real split
		{
			fixed transValue = trans->getAmount();			
			
			trans->addSplit(strDesc, strPayee, strCategory, fAmount);
			
			if (bShowNegAmountsInRed && !fAmount.IsPositive())
				bIsAmountNeg = YES;
			
			int nSplitsNumber = trans->getSplitCount() - 1;
			[m_SelectedTransaction setSplitTransaction:nSplitsNumber];
			[m_SelectedTransaction setIntValue:nSplitsNumber forKey:@"Split"];
			[m_SelectedTransaction setValue:[transactionsPayee stringValue] forKey:@"Payee"];
			[m_SelectedTransaction setValue:[transactionsDescription stringValue] forKey:@"Description"];
			[m_SelectedTransaction setValue:[transactionsCategory stringValue] forKey:@"Category"];
			[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
			[m_SelectedTransaction setBoolValue:bIsAmountNeg forKey:@"AmoNeg"];
			
			if (!strPayee.empty() && !m_Document.doesPayeeExist(strPayee))
			{
				m_Document.addPayee(strPayee);
				[transactionsPayee addItemWithObjectValue:[transactionsPayee stringValue]];
			}
			
			if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
			{
				m_Document.addCategory(strCategory);
				[transactionsCategory addItemWithObjectValue:[transactionsCategory stringValue]];
			}
			
			fixed splitValue = trans->getSplitTotal();
			
			fixed diff = transValue -= splitValue;
			
			// Then add a new dummy value if needed
			
			if (!diff.IsZero())
			{
				TransactionItem *transIndex = [m_aTransactionItems objectAtIndex:nTrans - m_nTransactionOffset];
				
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				NSNumber *nSAmount = [NSNumber numberWithDouble:diff.ToDouble()];
				NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
				
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:@"Split Value" forKey:@"Description"];
				[newSplit setValue:sSAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTrans];
				[newSplit setIntValue:nTrans forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:-2];
				[newSplit setIntValue:-2 forKey:@"Split"];
				
				[transIndex addChild:newSplit];
				
				[transactionsTableView reloadData];
				
				NSInteger row = [transactionsTableView rowForItem:newSplit];
				
				[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
				[transactionsTableView scrollRowToVisible:row];
				
				[window makeFirstResponder:transactionsPayee];
			}
			else
			{
				[transactionsTableView reloadData];
			}
		}		
	}
	
	[numberFormatter release];
	
	m_UnsavedChanges = true;
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
		
		TransactionType eType = schedTrans.getType();
		
		Date date1 = schedTrans.getNextDate();
		NSDate *datetemp = convertToNSDate(date1);
		
		int account = schedTrans.getAccount();
		
		[scheduledPayee setStringValue:[item valueForKey:@"payee"]];
		[scheduledCategory setStringValue:[item valueForKey:@"category"]];
		
		std::string strDescription = schedTrans.getDescription();
		NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
		[scheduledDescription setStringValue:sDescription];
		
		[scheduledAmount setStringValue:[item valueForKey:@"amount"]];
		
		Frequency eFreq = schedTrans.getFrequency();
		
		[scheduledFrequency selectItemAtIndex:eFreq];
		[scheduledType selectItemAtIndex:eType];
		
		Constraint eConstraint = schedTrans.getConstraint();
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
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLenient:YES];
	
	NSString *sScheduledAmount = [scheduledAmount stringValue];
	if ([[numberFormatter currencyCode] isEqualToString:@"USD"])
	{
		sScheduledAmount = [self convertUSNegAmount:sScheduledAmount];
	}
	
	NSNumber *nAmount = [numberFormatter numberFromString:sScheduledAmount];	
	fixed fAmount = [nAmount doubleValue];
	
	// reformat the number again, in case an abbreviation was used
	NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
	
	int nType = [scheduledType indexOfSelectedItem];
	TransactionType eType = static_cast<TransactionType>(nType);
	
	int nAccount = [scheduledAccount indexOfSelectedItem];
	
	int nFrequency = [scheduledFrequency indexOfSelectedItem];
	Frequency eFreq = static_cast<Frequency>(nFrequency);
	
	int nConstraint = [scheduledConstraint indexOfSelectedItem];
	Constraint eConstraint = static_cast<Constraint>(nConstraint);
	
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
	
	[numberFormatter release];
	
	[scheduledTransactionsTableView reloadData];
	
	m_UnsavedChanges = true;	
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
		
		m_UnsavedChanges = true;
		
		[self buildIndexTree];
	}	
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
	GraphType eType = static_cast<GraphType>(nType);
	
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
	GraphType eType = static_cast<GraphType>(nType);
	
	bool bIgnoreTransfers = false;
	
	if ([graphIgnoreTransfers state] == NSOnState)
		bIgnoreTransfers = true;
	
	int nDatePeriodSegment = [viewingPeriodSegmentControl selectedSegment];
	
	GraphDateType eDateType;
	
	switch (nDatePeriodSegment)
	{
		case 1:
			eDateType = DateWeek;
			break;
		case 2:
			eDateType = DateMonth;
			break;
		case 3:
			eDateType = DateYear;
			break;
		default:
			eDateType = DateCustom;
			break;
	}
	
	int nGraphViewType = [graphViewType indexOfTabViewItem:[graphViewType selectedTabViewItem]];
	GraphViewType eViewType = static_cast<GraphViewType>(nGraphViewType);

	int nItemsType = [graphItemTypes indexOfSelectedItem];
	GraphItemsType eItemsType = static_cast<GraphItemsType>(nItemsType);
	
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
	
	m_UnsavedChanges = true;
	
	[self redrawGraph:self];
}

- (IBAction)showRecentTransactions:(id)sender
{
	self.ShowTransactionsViewType = RECENT;
	[self buildTransactionsTree];
}

- (IBAction)showAllTransactionsThisYear:(id)sender
{
	self.ShowTransactionsViewType = ALL_THIS_YEAR;
	[self buildTransactionsTree];
}

- (IBAction)showAllTransactions:(id)sender
{
	self.ShowTransactionsViewType = ALL;
	[self buildTransactionsTree];
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
		
		m_UnsavedChanges = true;
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
		
		m_UnsavedChanges = true;
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
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];

	NSMutableDictionary *item = [[NSMutableDictionary alloc] init];
	
	std::string strSPayee = newST.getPayee();
	NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
	
	std::string strSCategory = newST.getCategory();
	NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
	
	NSNumber *nSAmount = [NSNumber numberWithDouble:newST.getAmount().ToDouble()];
	
	NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
	
	NSString *sSDate = [[dateFormatter stringFromDate:ndate1] retain];	
	
	[item setValue:[NSNumber numberWithInt:schedTransIndex] forKey:@"index"];
	[item setValue:sSPayee forKey:@"payee"];
	[item setValue:sSCategory forKey:@"category"];
	[item setValue:sSAmount forKey:@"amount"];
	[item setValue:sSDate forKey:@"nextdate"];
	[item setValue:[NSNumber numberWithBool:true] forKey:@"enabled"];
	
	[m_aScheduledTransactions addObject:item];
	
	[dateFormatter release];
	[numberFormatter release];
	
	[scheduledTransactionsTableView reloadData];
	
	NSInteger row = schedTransIndex;
	
	[scheduledTransactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	[scheduledTransactionsTableView scrollRowToVisible:row];
	
	[window makeFirstResponder:scheduledPayee];
	
	m_UnsavedChanges = true;
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
	m_UnsavedChanges = true;
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
	
	newGraph.setStartDate(currentDate);
	
	currentDate.setDay(1);
	newGraph.setEndDate(currentDate);
	
	int nGraphNum = m_Document.addGraph(newGraph);
	
	NSString *sGraphKey = [NSString stringWithFormat:@"g@s", nGraphNum];
	
	[indexBar addItem:@"graphs" key:sGraphKey title:sNewGraph item:nGraphNum action:@selector(graphSelected:) target:self type:3 rename:@selector(graphRenamed:) renameTarget:self];
	
	m_UnsavedChanges = true;
	
	[indexBar reloadData];
	
	[indexBar renameItem:sGraphKey];
}

// Transactions OutlineView Start

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if (item == nil)
	{
		return [m_aTransactionItems objectAtIndex:index];
	}
	else
	{
		return [(TransactionItem*)item childAtIndex:index];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [item expandable];
}

-(NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (item == nil)
	{
		if (outlineView == transactionsTableView)
		{
			return [m_aTransactionItems count];
		}
    }
	
    return [item childrenCount];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	NSString *identifier = [tableColumn identifier];
	
	if ([identifier caseInsensitiveCompare:@"cleared"] == NSOrderedSame)
	{
		int nCleared = [item intKeyValue:identifier];
		return [NSNumber numberWithInt:nCleared];
	}

    return [item keyValue:identifier];
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	NSString *identifier = [tableColumn identifier];
	
	if (item)
	{
		if (outlineView == transactionsTableView)
		{
			[item setValue:object forKey:identifier];
			
			int nTrans = [item transaction];
	
			Transaction *trans = NULL;
			
			if (nTrans >= 0)
				trans = &m_pAccount->getTransaction(nTrans);
			
			if ([identifier isEqualToString:@"Cleared"])
			{
				if ([object boolValue] == YES)
				{
					trans->setCleared(true);
				}
				else
				{
					trans->setCleared(false);
				}
				
				m_UnsavedChanges = true;
			}
		}
	}
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	BOOL result = NO;
	
	return result;
}

- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	if (outlineView != transactionsTableView)
	{
		return;
	}
	
	NSColor *fontColor;
	
	BOOL enableCheck = YES;
	
	if ([item intKeyValue:@"Split"] != -1)
		enableCheck = NO;
	
	if ([[tableColumn identifier] isEqualToString:@"Cleared"])
	{
		if (enableCheck == YES)
		{
			[cell setImagePosition: NSImageOnly];
		}
		else
		{
			[cell setImagePosition: NSNoImage];
		}
		
		return;
	}
		
	if ([outlineView selectedRow] == [outlineView rowForItem:item])
	{
		fontColor = [NSColor whiteColor];
	}
	else
	{
		if ([item intKeyValue:@"Split"] == -2)
		{
			fontColor = [NSColor grayColor];
		}
		else
		{
			fontColor = [NSColor blackColor];
		}
	}	
	
	if ([[tableColumn identifier] isEqualToString:@"Amount"])
	{
		if ([item boolKeyValue:@"AmoNeg"] == YES)
		{
			fontColor = [NSColor redColor];
		}		
	}
	else if ([[tableColumn identifier] isEqualToString:@"Balance"])
	{
		if ([item boolKeyValue:@"BalNeg"] == YES)
		{
			fontColor = [NSColor redColor];
		}		
	}
	
	[cell setTextColor:fontColor];
}

// Transactions OutlineView End

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
	else if (aTableView == graphItemsTableView)
	{
		NSString *sItem = [m_aGraphItems objectAtIndex:rowIndex];
		
		result = sItem;
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
	else if (aTableView == graphItemsTableView)
		return [m_aGraphItems count];
	
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
			
			m_UnsavedChanges = true;
		}			
	}
	else if (aTableView == graphItemsTableView)
	{
		NSString *sItem = [m_aGraphItems objectAtIndex:rowIndex];
		
		sItem = object;
		
		[m_aGraphItems replaceObjectAtIndex:rowIndex withObject:sItem];
		
		[self redrawGraph:self];
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
			
			m_UnsavedChanges = true;
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
			
			m_UnsavedChanges = true;
		}
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

// Payees/Categories TableView End

NSDate *convertToNSDate(Date &date)
{
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
    
    NSDateComponents *dateComponents = [[NSDateComponents alloc] init];
    [dateComponents setYear:date.getYear()];
    [dateComponents setMonth:date.getMonth()];
    [dateComponents setDay:date.getDay()];
    
    [dateComponents setHour:0];
    [dateComponents setMinute:0];
    [dateComponents setSecond:0];
	
	NSDate *nsDate = [gregorian dateFromComponents:dateComponents];
//	[dateComponents release];
//	[gregorian release];
	
    return nsDate;
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
	//	[dateComponents release];
	//	[gregorian release];
	
    return nsDate;	
}

- (IBAction)NewFile:(id)sender
{
	if (m_UnsavedChanges)
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
	
	m_UnsavedChanges = false;
	m_DocumentFile = "";
	
	m_SelectedTransaction = 0;
	m_pAccount = 0;
	
	m_Document.clear();
	
	[self setWindowTitleWithDocName:nil];
	
	[self buildIndexTree];
	[self buildTransactionsTree];
	[self buildSchedTransList];
	[self refreshLibraryItems];
}

- (IBAction)OpenFile:(id)sender
{
	if (m_UnsavedChanges)
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
		fileToOpen = [oPanel filename];
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
		m_UnsavedChanges = false;
		
		m_SelectedTransaction = 0;
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
			m_UnsavedChanges = false;
		}
	}
}

- (IBAction)SaveFileAs:(id)sender
{
	NSSavePanel *sPanel = [NSSavePanel savePanel];
	NSString *fileToSave;
	std::string strFile = "";
	
	[sPanel setTitle:NSLocalizedString(@"Save Document", "Save Document Title")];
	[sPanel setRequiredFileType:@"stash"];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"stash", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
		fileToSave = [sPanel filename];
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
			m_UnsavedChanges = false;
		}
	}
}

- (bool)OpenFileAt:(std::string)path
{
	std::fstream fileStream(path.c_str(), std::ios::in | std::ios::binary);
	
	if (!fileStream)
	{
		return false;;
	}
	
	if (!m_Document.Load(fileStream))
		return false;
	
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
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
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
			
			NSNumber *nSAmount = [NSNumber numberWithDouble:it->getAmount().ToDouble()];
			
			NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
			
			NSDate *date = convertToNSDate(const_cast<Date&>(it->getNextDate2()));
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
		}
	}
	
	[numberFormatter release];
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
	
	Account &oAccount = m_Document.getAccount(schedTrans.getAccount());
	
	oAccount.addTransaction(newTransaction);
	
	[self buildTransactionsTree];
	
	schedTrans.AdvanceNextDate();
	
	m_UnsavedChanges = true;
}

- (void)SkipDueScheduledTransaction:(int)index
{
	ScheduledTransaction &schedTrans = m_Document.getScheduledTransaction(index);
	
	schedTrans.AdvanceNextDate();
	
	m_UnsavedChanges = true;
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
	[oPanel setRequiredFileType:@"qif"];
	[oPanel setTitle:NSLocalizedString(@"Import QIF file", "Import QIF Title")];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"qif", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
		fileToOpen = [oPanel filename];
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
	DateStringFormat dateFormat = static_cast<DateStringFormat>(nDateFormat);
	
	char cSeparator = [importQIFController separator];
	
	BOOL bMarkCleared = [importQIFController markAsCleared];
	
	NSString *sFile = [importQIFController file];
		
	std::string strFile = [sFile cStringUsingEncoding: NSUTF8StringEncoding];
	
	[importQIFController release];
	
	bool bMC = true;
	if (bMarkCleared == NO)
		bMC = false;
	
	if (importQIFFileToAccount(m_pAccount, strFile, dateFormat, cSeparator, bMC))
		[self buildTransactionsTree];
}

- (IBAction)ImportOFX:(id)sender
{
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	NSString *fileToOpen;
	std::string strFile = "";
	[oPanel setAllowsMultipleSelection:NO];
	[oPanel setResolvesAliases:YES];
	[oPanel setRequiredFileType:@"ofx"];
	[oPanel setTitle:NSLocalizedString(@"Select OFX file to Import", "Import OFX Title")];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"ofx", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
		fileToOpen = [oPanel filename];
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
		
		NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
		[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
		
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
				const Date &dtStart = stResp.getTransaction1(0).getDate1();				
				const Date &dtEnd = stResp.getTransaction1(nMaxLook).getDate1();
				
				if (dtStart > dtEnd)
				{
					bFoundReverse = true;
					bReverseTransactions = YES;
				}				
			}
			
			NSString *sAccountNumber = [[NSString alloc] initWithUTF8String:bankAccount.getAccountID().c_str()];		
			NSString *sTransactionCount = [NSString stringWithFormat:@"%i", stResp.getTransactionCount()];
			
			fixed balance = stResp.getBalance();
			
			NSNumber *nSBalance = [NSNumber numberWithDouble:balance.ToDouble()];
			NSString *sBalance = [[numberFormatter stringFromNumber:nSBalance] retain];
						
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
		
		[numberFormatter release];		
		
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
	
	m_UnsavedChanges = true;
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
	[sPanel setRequiredFileType:@"ofx"];
	[sPanel setTitle:NSLocalizedString(@"Select OFX file to Export to", "Export OFX Title")];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"ofx", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
		fileToSave = [sPanel filename];
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
	[sPanel setRequiredFileType:@"qif"];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"qif", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
		fileToSave = [sPanel filename];
		strFile = [fileToSave cStringUsingEncoding: NSUTF8StringEncoding];
		
		exportAccountToQIFFile(m_pAccount, strFile, UK);
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
	
	if (action == @selector(MoveUp:) || action == @selector(MoveDown:) || action == @selector(SplitTransaction:))
	{
		if (!m_pAccount || !m_SelectedTransaction)
			return NO;
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
	if (m_UnsavedChanges)
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
	
	// Save Transactions view OutlineView column sizes
	
	NSMutableDictionary *visibleTransactionColumns = [NSMutableDictionary dictionary];
	NSMutableDictionary *transactionColumnSizes = [NSMutableDictionary dictionary];
	
	NSMutableArray *transactionColumnOrder = [NSMutableArray array];
	
	for (NSTableColumn *tc in [transactionsTableView tableColumns])
	{		
		BOOL bIsVisible = ![tc isHidden];
		[visibleTransactionColumns setObject:[NSNumber numberWithBool:bIsVisible] forKey:[tc identifier]];
		
		if (bIsVisible)
		{
			float fWidth = [tc width];
			[transactionColumnSizes setObject:[NSNumber numberWithFloat:fWidth] forKey:[tc identifier]];
		}
		
		[transactionColumnOrder addObject:[tc identifier]];
	}
	
	[defs setObject:visibleTransactionColumns forKey:@"VisibleTransactionColumns"];
	[defs setObject:transactionColumnSizes forKey:@"TransactionColumnSizes"];
	[defs setObject:transactionColumnOrder forKey:@"TransactionColumnOrder"];
	
	// Save the splitter positions
	[indexBarSplitView saveLayoutToDefault:@"indexSplitter"];
	[transactionsverticalSplitView saveLayoutToDefault:@"transactionsSplitter"];
	[scheduledverticalSplitView saveLayoutToDefault:@"schedtransSplitter"];
	
	[defs setInteger:self.ShowTransactionsViewType forKey:@"TransactionsViewType"];
	[defs synchronize];
	
	[m_aTransactionItems release];
	[m_aPayeeItems release];
	[m_aCategoryItems release];
	[m_aScheduledTransactions release];
	[m_aGraphItems release];
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
	
	if (m_UnsavedChanges)
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
	m_UnsavedChanges = false;
	
	[self buildIndexTree];
	[self refreshLibraryItems];
	
	[self calculateAndShowScheduled];

	return YES;
}

- (void)transactionTableColumnMenu:(id)sender
{
	if ([sender state] == NSOnState)
	{
		[sender setState:NSOffState];
		[[sender representedObject] setHidden:TRUE];
	}
	else
	{
		[sender setState:NSOnState];
		[[sender representedObject] setHidden:FALSE];
	}
}

- (NSString*)transactionTypeToString:(TransactionType)type
{
	NSString *string;
	
	switch (type)
	{
		case None:
			string = NSLocalizedString(@"None", "Transaction Type -> None");
			break;
		case Deposit:
			string = NSLocalizedString(@"Deposit", "Transaction Type -> Deposit");
			break;
		case Withdrawal:
			string = NSLocalizedString(@"Withdrawal", "Transaction Type -> Withdrawal");
			break;
		case Transfer:
			string = NSLocalizedString(@"Transfer", "Transaction Type -> Transfer");
			break;
		case StandingOrder:
			string = NSLocalizedString(@"Standing Order", "Transaction Type -> Standing Order");
			break;
		case DirectDebit:
			string = NSLocalizedString(@"Direct Debit", "Transaction Type -> Direct Debit");
			break;
		case PointOfSale:
			string = NSLocalizedString(@"Point Of Sale", "Transaction Type -> Point Of Sale");
			break;
		case Charge:
			string = NSLocalizedString(@"Charge", "Transaction Type -> Charge");
			break;
		case ATM:
			string = NSLocalizedString(@"ATM", "Transaction Type -> ATM");
			break;
		case Cheque:
			string = NSLocalizedString(@"Check", "Transaction Type -> Check");
			break;
		case Credit:
			string = NSLocalizedString(@"Credit", "Transaction Type -> Credit");
			break;
		case Debit:
			string = NSLocalizedString(@"Debit", "Transaction Type -> Debit");
			break;
	}
	
	return string;	
}

- (NSString*)convertUSNegAmount:(NSString*)string
{
	NSRange r;
	
	r = [string rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"-"]];
	
	if (r.length == 0)
	{
		return string;
	}
	
	r = [string rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"("]];
	
	if (r.length != 0)
	{
		return string;
	}
	
	NSMutableString *mutableString = [NSMutableString stringWithString:string];
	
	r = [mutableString rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"-"]];
	
	[mutableString deleteCharactersInRange:r];
	
	NSString *newString = [NSString stringWithFormat:@"(%@)", mutableString];
	
	return newString;	
}

// update the balances of all transactions from the given index down
- (void)updateBalancesFromTransactionIndex:(int)nIndex
{
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	fixed localBalance = 0.0;
	
	if (nIndex > m_aBalance.size())
	{
		nIndex = m_aBalance.size();
	}
	
	m_aBalance.erase(m_aBalance.begin() + nIndex, m_aBalance.end());
	
	if (nIndex == 0)
	{
		localBalance = m_pAccount->getBalance(true, m_nTransactionOffset);
	}
	else
	{
		localBalance = m_aBalance.back();
	}
	
	BOOL bShowNegBalancesInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegBalancesRed"];
	
	std::vector<Transaction>::iterator it = m_pAccount->begin() + m_nTransactionOffset + nIndex;
	
	int nTransItemIndex = nIndex;
	for (; it != m_pAccount->end(); ++it, nTransItemIndex++)
	{
		TransactionItem *aTransaction = [m_aTransactionItems objectAtIndex:nTransItemIndex];
		
		localBalance += it->getAmount();
		
		m_aBalance.push_back(localBalance);
		
		BOOL bIsBalanceNeg = NO;		
		
		if (bShowNegBalancesInRed && !localBalance.IsPositive())
			bIsBalanceNeg = YES;
		
		NSNumber *nTBalance = [NSNumber numberWithDouble:localBalance.ToDouble()];
		NSString *sTBalance = [[numberFormatter stringFromNumber:nTBalance] retain];
				
		[aTransaction setValue:sTBalance forKey:@"Balance"];
		
		[aTransaction setBoolValue:bIsBalanceNeg forKey:@"BalNeg"];
	
		[sTBalance release];
	}	

	[numberFormatter release];
}

// update the transaction indexes after a deletion
- (void)updateTransactionsFromTransactionIndex:(int)nIndex
{
	int nTotalItems = [m_aTransactionItems count];
	
	int nTransItemIndex = nIndex;
	for (; nTransItemIndex < nTotalItems; nTransItemIndex++)
	{
		TransactionItem *aTransaction = [m_aTransactionItems objectAtIndex:nTransItemIndex];
		
		int nTransIndex = [aTransaction transaction];
		
		nTransIndex--;
		
		[aTransaction setTransaction:nTransIndex];
		[aTransaction setIntValue:nTransIndex forKey:@"Transaction"];
	}	
}

- (void)handleTransactionsSettingsUpdate:(NSNotification *)note
{
	[self buildTransactionsTree];
}

- (void)handleGraphSettingsUpdate:(NSNotification *)note
{
	[self redrawGraph:self];	
}

- (IBAction)gotoWebsite:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://wiki.github.com/ppearson/Stash"]];
}

- (IBAction)reportBug:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://stash.lighthouseapp.com/"]];
}

@end
