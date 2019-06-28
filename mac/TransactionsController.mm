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

#import "TransactionsController.h"

#include "document.h"
#include "account.h"

#import "NSDateEx.h"
#import "SplitViewEx.h"
#import "IndexItem.h"
#import "ValueFormatter.h"

@implementation TransactionsController

static TransactionsController *gSharedInterface = nil;

+ (TransactionsController*)sharedInterface
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
	if (self)
	{
		transactionsTableHeaderMenu = [[NSMenu alloc] initWithTitle:@"Menu"];
		
		NSNotificationCenter *nc;
		nc = [NSNotificationCenter defaultCenter];
		[nc addObserver:self selector:@selector(handleTransactionsSettingsUpdate:) name:@"TransactionsSettingsUpdate" object:nil];
		
		m_pDocument = NULL;
	}
	
	return self;
}

- (void)dealloc
{
	[transactionsTableHeaderMenu release];
	
	NSNotificationCenter *nc;
	nc = [NSNotificationCenter defaultCenter];
	[nc removeObserver:self];
	
	[super dealloc];
}

- (void)setMainController:(StashAppDelegate *)controller
{
	fMainController = controller;
}

- (void)awakeFromNib
{
	m_bEditing = false;	
	m_SelectedTransaction = 0;
	
	NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
	
	int nTransactionsViewType = [[defs objectForKey:@"TransactionsViewType"] intValue];
	
	if (nTransactionsViewType >= 0 && nTransactionsViewType < 3)
	{
		m_showTransactionsViewType = nTransactionsViewType;
	}
	else
	{
		m_showTransactionsViewType = RECENT;
	}
	
	[transactionsverticalSplitView loadLayoutFromDefault:@"transactionsSplitter"];
	
	m_aTransactionItems = [[NSMutableArray alloc] init];
	
	// Load Transactions view OutlineView column sizes
	
	NSDictionary *visibleTransactionColumns = [[NSUserDefaults standardUserDefaults] objectForKey:@"VisibleTransactionColumns"];
	NSDictionary *transactionColumnSizes = [[NSUserDefaults standardUserDefaults] objectForKey:@"TransactionColumnSizes"];
	NSMutableArray *transactionColumnOrder = [[NSUserDefaults standardUserDefaults] objectForKey:@"TransactionColumnOrder"];
	
	[[transactionsTableView headerView] setMenu:transactionsTableHeaderMenu];
    
//    [transactionsTableView setFont:<#(NSFont * _Nullable)#>]
	
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

	[transactionsTableView setDelegate:self];
	[transactionsTableView setAutoresizesOutlineColumn:NO];	
	
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(TransactionSelectionDidChange:) name:NSOutlineViewSelectionDidChangeNotification object:transactionsTableView];
}

- (void)showTransactionsForAccount:(Account*)account
{
	m_pAccount = account;
	
	[transactionsPayee setStringValue:@""];
	[transactionsDescription setStringValue:@""];
	[transactionsCategory setStringValue:@""];
	[transactionsAmount setStringValue:@""];
	[transactionsType selectItemAtIndex:0];
	[transactionsCleared setState:NSOffState];
	
	[transactionsTableView deselectAll:self];
	
	m_pDocument = Document::getInstance();
	
	[self buildTransactionsTree];
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
	std::vector<Transaction>::const_iterator itEnd = m_pAccount->end();
	
	unsigned int nTransaction = 0;
	m_nTransactionOffset = 0;
	
	if (m_showTransactionsViewType != ALL)
	{
		Date dateCompare;
		dateCompare.Now();
		
		if (m_showTransactionsViewType == RECENT)
		{
			int nRecentDuration = [[NSUserDefaults standardUserDefaults] integerForKey:@"TransactionsRecentDuration"];
			
			if (nRecentDuration <= 0)
				nRecentDuration = 30;
			
			dateCompare.DecrementDays(nRecentDuration);
		}
		else // all this year
		{
			dateCompare.setDay(1);
			dateCompare.setMonth(1);
		}
		
		std::vector<Transaction>::const_iterator itTemp = m_pAccount->begin();
		std::vector<Transaction>::const_iterator itTempEnd = m_pAccount->end();
		
		for (; itTemp != itTempEnd; ++itTemp)
		{
			if ((*itTemp).getDate1() >= dateCompare)
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
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	BOOL bShowNegAmountsInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegAmountsRed"];
	BOOL bShowNegBalancesInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegBalancesRed"];
	
	for (; it != itEnd; ++it, nTransaction++)
	{
		TransactionItem *newTransaction = [[TransactionItem alloc] init];
		
		std::string strTPayee = it->getPayee();
		NSString *sTPayee = [[NSString alloc] initWithUTF8String:strTPayee.c_str()];
		
		std::string strTDescription = it->getDescription();
		NSString *sTDescription = [[NSString alloc] initWithUTF8String:strTDescription.c_str()];
		
		std::string strTCategory = it->getCategory();
		NSString *sTCategory = [[NSString alloc] initWithUTF8String:strTCategory.c_str()];
		
		NSString *sTType = [self transactionTypeToString:it->getType()];
		
		NSString *sTAmount = [[valueFormatter currencyStringFromFixed:it->getAmount()] retain];
		
		NSDate *date = convertToNSDate(const_cast<Date&>(it->getDate1()));
		NSString *sTDate = [[dateFormatter stringFromDate:date] retain];
		
		localBalance += it->getAmount();
		
		m_aBalance.push_back(localBalance);
		
		NSString *sTBalance = [[valueFormatter currencyStringFromFixed:localBalance] retain];
		
		[newTransaction setTransaction:nTransaction];
		
		int cleared = it->isCleared();
		
		BOOL bIsAmountNeg = NO;
		BOOL bIsBalanceNeg = NO;
		
		if (bShowNegAmountsInRed && !it->getAmount().IsPositive())
			bIsAmountNeg = YES;
		
		if (bShowNegBalancesInRed && !localBalance.IsPositive())
			bIsBalanceNeg = YES;
		
		[newTransaction setIntValue:cleared forKey:@"Cleared"];
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
			unsigned int nSplits = it->getSplitCount();
			
			fixed splitValue = it->getAmount();
			
			for (unsigned int i = 0; i < nSplits; i++)
			{
				const SplitTransaction &split = it->getSplit(i);
				
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				std::string strSPayee = split.getPayee();
				NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
				
				std::string strSDescription = split.getDescription();
				NSString *sSDescription = [[NSString alloc] initWithUTF8String:strSDescription.c_str()];
				
				std::string strSCategory = split.getCategory();
				NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
				
				NSString *sSAmount = [[valueFormatter currencyStringFromFixed:split.getAmount()] retain];
				
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
				[newSplit release];
				
				[sSPayee release];
				[sSDescription release];
				[sSCategory release];
				[sSAmount release];
			}
			
			// add remainder as a temp editable row
			
			if (!splitValue.IsZero())
			{
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				NSString *sSAmount = [[valueFormatter currencyStringFromFixed:splitValue] retain];
				
				[newSplit setValue:@"Split Value" forKey:@"Description"];
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:sSAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:-2];
				[newSplit setIntValue:-2 forKey:@"Split"];
				
				[newTransaction addChild:newSplit];
				[sSAmount release];
				[newSplit release];
			}
		}		
		
		[m_aTransactionItems addObject:newTransaction];
		[newTransaction release];
		[sTPayee release];
		[sTDescription release];
		[sTCategory release];
		[sTAmount release];
		[sTDate release];
		[sTBalance release];
	}
	
	[dateFormatter release];
	
	[transactionsTableView reloadData];
	
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsScrollToLatest"] == YES)
	{
		int latest = [transactionsTableView numberOfRows];
		[transactionsTableView scrollRowToVisible:latest - 1];
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
    
    BOOL bTransactionsAreClearedByDefault = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsAreClearedByDefault"];
	
	Transaction newTransaction("", "", "", 0.0, date1);
    
    if (bTransactionsAreClearedByDefault)
    {
        newTransaction.setCleared(true);
    }
	
	int nTransaction = m_pAccount->addTransaction(newTransaction);
	
	[transactionsPayee setStringValue:@""];
	[transactionsDescription setStringValue:@""];
	[transactionsCategory setStringValue:@""];
	[transactionsAmount setStringValue:@""];
	[transactionsType selectItemAtIndex:0];
    
    [transactionsCleared setState:(bTransactionsAreClearedByDefault ? NSOnState : NSOffState)];
	
	TransactionItem *newIndex = [self createTransactionItem:newTransaction index:nTransaction];
	
	[m_aTransactionItems addObject:newIndex];
	[newIndex release];
	
	[transactionsTableView reloadData];
	
	NSInteger row = [transactionsTableView rowForItem:newIndex];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	[transactionsTableView scrollRowToVisible:row];
	
	[window makeFirstResponder:transactionsPayee];
	
	[self setDocumentModified:TRUE];
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
		
		[self setDocumentModified:TRUE];
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
	
	if (!trans.isSplit())
	{
		fixed splitValue = trans.getAmount();
		TransactionItem *newSplit = [[TransactionItem alloc] init];
		
		ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
		
		NSString *sAmount = [[valueFormatter currencyStringFromFixed:splitValue] retain];
		
		[newSplit setValue:@"Split Value" forKey:@"Description"];
		[newSplit setValue:@"Split Value" forKey:@"Payee"];
		[newSplit setValue:sAmount forKey:@"Amount"];
		
		[newSplit setTransaction:nTransaction];
		[newSplit setIntValue:nTransaction forKey:@"Transaction"];
		
		[newSplit setSplitTransaction:-2];
		[newSplit setIntValue:-2 forKey:@"Split"];
		
		[item addChild:newSplit];
		
		[transactionsTableView reloadData];
		
		[self setDocumentModified:TRUE];
		
		[transactionsTableView expandItem:item];
		
		row = [transactionsTableView rowForItem:newSplit];
	}
	else
	{
		[transactionsTableView expandItem:item];
		
		int nSplits = trans.getSplitCount();
		row += nSplits;
		
		fixed remainderAmount = trans.getAmount() - trans.getSplitTotal();
		if (!remainderAmount.IsZero())
		{
			row += 1;
		}
	}
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	[transactionsTableView scrollRowToVisible:row];
	
	[window makeFirstResponder:transactionsPayee];
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

- (void)SwapTransactions:(unsigned int)from to:(unsigned int)to
{
	if (from == -1u || to == -1u)
		return;
	
	unsigned int top = m_pAccount->getTransactionCount();
	
	if (from >= top || to >= top)
		return;
	
	unsigned int nItemFrom = from - m_nTransactionOffset;
	unsigned int nItemTo = to - m_nTransactionOffset;
	
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
	
	unsigned int nNewRow = [transactionsTableView rowForItem:toItem];
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:nNewRow] byExtendingSelection:NO];
	[transactionsTableView scrollRowToVisible:nNewRow];
	
	[self setDocumentModified:TRUE];
}

- (IBAction)RefreshView:(id)sender
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
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	NSString *sAmount = [[valueFormatter currencyStringFromFixed:transaction.getAmount()] retain];
	
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
	
	NSString *sBalance = [[valueFormatter currencyStringFromFixed:localBalance] retain];
	
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
	
    [sPayee release];
    [sDescription release];
    [sCategory release];
    [sType release];
    [sBalance release];
    
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
		
		ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
		
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
			
			NSString *sAmount = [[valueFormatter currencyStringFromFixed:trans->getAmount()] retain];
			
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
			
			NSString *sAmount = [[valueFormatter currencyStringFromFixed:split->getAmount()] retain];
			
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
			
			[window makeFirstResponder:transactionsPayee];
			
			[transactionsCleared setEnabled:NO];
			[transactionsType setEnabled:NO];
			[transactionsDateCntl setEnabled:NO];
		}
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
	
	NSString* sTransactionAmount = [transactionsAmount stringValue];
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	fixed fAmount = [valueFormatter fixedFromString:sTransactionAmount];
	
	int nTrans = [m_SelectedTransaction transaction];
	int nSplit = [m_SelectedTransaction splitTransaction];
	
	Transaction *trans = NULL;
	SplitTransaction *split = NULL;
	
	if (nTrans != -1)
		trans = &m_pAccount->getTransaction(nTrans);
	
	if (nSplit >= 0)
		split = &trans->getSplit(nSplit);
	
	int nType = [transactionsType indexOfSelectedItem];
	TransactionType eType = static_cast<TransactionType>(nType);
	
	NSString *sType = [self transactionTypeToString:eType];
	
	// if it's a Normal transaction
	if (trans && !split && nSplit != -2)
	{
		BOOL bEnforceNegativeCategories = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsEnforceNegForCategories"];
		if (bEnforceNegativeCategories)
		{
			// if type is a negative type, make sure amount is negative
			bool bNegType = false;
			
			switch (eType)
			{
				case Withdrawal:
				case PointOfSale:
				case Debit:
				case ATM:
					bNegType = true;
					break;
				default:
					break;
			}
			
			if (bNegType)
				fAmount.setNegative();
		}
	}
	else // split
	{
		BOOL bEnforceNegativeSplits = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsEnforceNegForSplit"];
		if (bEnforceNegativeSplits)
		{
			// if its Parent Transaction amount is Negative, make sure its amount is negative
			
			fixed transactionAmount = trans->getAmount();
			if (!transactionAmount.IsPositive())
				fAmount.setNegative();
		}
	}
	
	// reformat the number again, in case an abbreviation was used, or we've made it negative
	NSString *sAmount = [[valueFormatter currencyStringFromFixed:fAmount] retain];
	
	bool bCleared = false;
	
	if ([transactionsCleared state] == NSOnState)
		bCleared = true;
	
	BOOL bIsAmountNeg = NO;
	
	BOOL bShowNegAmountsInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegAmountsRed"];
	
	Document* pDocument = Document::getInstance();
	
	if (!pDocument)
	{
		
	}
	
	bool bBlankSplitAdded = false;
	TransactionItem *newSplit = NULL;
	
	if (trans && !split && nSplit != -2) // normal transactions
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
		[m_SelectedTransaction setValue:sType forKey:@"Type"];
	}
	else
	{
		if (nSplit != -2) // was an existing Split Transaction
		{
			split->setPayee(strPayee);
			split->setDescription(strDesc);
			split->setCategory(strCategory);
			split->setAmount(fAmount);
			
			if (bShowNegAmountsInRed && !fAmount.IsPositive())
				bIsAmountNeg = YES;
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
			
			fixed splitValue = trans->getSplitTotal();
			
			fixed diff = transValue -= splitValue;
			
			// Then add a new dummy value if needed
			if (!diff.IsZero())
			{
				TransactionItem *transIndex = [m_aTransactionItems objectAtIndex:nTrans - m_nTransactionOffset];
				
				newSplit = [[TransactionItem alloc] init];
				
				NSString *sSAmount = [[valueFormatter currencyStringFromFixed:diff] retain];
				
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:@"Split Value" forKey:@"Description"];
				[newSplit setValue:sSAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTrans];
				[newSplit setIntValue:nTrans forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:-2];
				[newSplit setIntValue:-2 forKey:@"Split"];
				
				[transIndex addChild:newSplit];
				
				bBlankSplitAdded = true;
			}
		}		
	}
	
	[m_SelectedTransaction setValue:[transactionsPayee stringValue] forKey:@"Payee"];
	[m_SelectedTransaction setValue:[transactionsDescription stringValue] forKey:@"Description"];
	[m_SelectedTransaction setValue:[transactionsCategory stringValue] forKey:@"Category"];
	[m_SelectedTransaction setIntValue:bCleared forKey:@"Cleared"];
	[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
	[m_SelectedTransaction setBoolValue:bIsAmountNeg forKey:@"AmoNeg"];
	
	if (!strPayee.empty() && !pDocument->doesPayeeExist(strPayee))
	{
		pDocument->addPayee(strPayee);
		[transactionsPayee addItemWithObjectValue:[transactionsPayee stringValue]];
	}
	
	if (!strCategory.empty() && !pDocument->doesCategoryExist(strCategory))
	{
		pDocument->addCategory(strCategory);
		[transactionsCategory addItemWithObjectValue:[transactionsCategory stringValue]];
	}
	
	[transactionsTableView reloadData];
	
	[self setDocumentModified:TRUE];
	
	if (bBlankSplitAdded)
	{
		unsigned int newSplitIndex = [transactionsTableView rowForItem:newSplit];
		
		[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:newSplitIndex] byExtendingSelection:NO];
		[transactionsTableView scrollRowToVisible:newSplitIndex];
		
		[window makeFirstResponder:transactionsPayee];
	}
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
            
            TransactionItem* transItem = (TransactionItem*)item;
			
			int nTrans = [transItem transaction];
			
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
				
				// need to update the balance in the IndexBar
				[self updateBalancesFromTransactionIndex:nTrans - m_nTransactionOffset];
				
				[self setDocumentModified:TRUE];
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
	[dateComponents release];
	[gregorian release];
	
    return nsDate;
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

// update the balances of all transactions from the given index down
- (void)updateBalancesFromTransactionIndex:(unsigned int)nIndex
{
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
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
	
	fixed clearedBalance = localBalance;
	
	BOOL bShowNegBalancesInRed = [[NSUserDefaults standardUserDefaults] boolForKey:@"TransactionsNegBalancesRed"];
	
	std::vector<Transaction>::iterator it = m_pAccount->begin() + m_nTransactionOffset + nIndex;
	std::vector<Transaction>::iterator itEnd = m_pAccount->end();
	
	int nTransItemIndex = nIndex;
	for (; it != itEnd; ++it, nTransItemIndex++)
	{
		TransactionItem *aTransaction = [m_aTransactionItems objectAtIndex:nTransItemIndex];
		
		localBalance += it->getAmount();
		
		if (it->isCleared())
			clearedBalance += it->getAmount();
		
		m_aBalance.push_back(localBalance);
		
		BOOL bIsBalanceNeg = NO;		
		
		if (bShowNegBalancesInRed && !localBalance.IsPositive())
			bIsBalanceNeg = YES;
		
		NSString *sTBalance = [[valueFormatter currencyStringFromFixed:localBalance] retain];
		
		[aTransaction setValue:sTBalance forKey:@"Balance"];
		
		[aTransaction setBoolValue:bIsBalanceNeg forKey:@"BalNeg"];
		
		[sTBalance release];
	}
	
	// need to update IndexBar account balance
	
	NSString *sTBalance = [[valueFormatter currencyStringFromFixed:clearedBalance] retain];
	
	NSString *itemKey = [indexBar getSelectedItemKey];
	
	if (itemKey)
	{
		[indexBar updateAmount:itemKey amount:sTBalance];
		
		[indexBar reloadData];
	}
    
    [sTBalance release];
}

// update the transaction indexes after a deletion
- (void)updateTransactionsFromTransactionIndex:(unsigned int)nIndex
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

- (void)cleanUp
{
	NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
	
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
	
	[transactionsverticalSplitView saveLayoutToDefault:@"transactionsSplitter"];
	
	[defs setInteger:m_showTransactionsViewType forKey:@"TransactionsViewType"];
	
	[defs synchronize];
	
	[m_aTransactionItems removeAllObjects];
	
	[m_aTransactionItems release];
	
}

- (void)setWindow:(NSWindow*)wnd
{
	window = wnd;
}

- (void)setIndexBar:(IndexBar*)bar
{
	indexBar = bar;
}

- (void)refreshLibraryItems
{
	Document* pDocument = Document::getInstance();
	if (!pDocument)
		return;
	
	[transactionsPayee removeAllItems];
	
	std::set<std::string>::iterator it = pDocument->PayeeBegin();
	
	for (; it != pDocument->PayeeEnd(); ++it)
	{
		NSString *sPayee = [[NSString alloc] initWithUTF8String:(*it).c_str()];
		
		[transactionsPayee addItemWithObjectValue:sPayee];
		
		[sPayee release];
	}
	
	[transactionsCategory removeAllItems];
	
	std::set<std::string>::iterator itCat = pDocument->CategoryBegin();
	
	for (; itCat != pDocument->CategoryEnd(); ++itCat)
	{
		NSString *sCategory = [[NSString alloc] initWithUTF8String:(*itCat).c_str()];
		
		[transactionsCategory addItemWithObjectValue:sCategory];
		
		[sCategory release];
	}
}

- (void)makeTransferItem:(MakeTransferController *)makeTransferCont
{
	int nFromAccount = [makeTransferCont fromAccount];
	int nToAccount = [makeTransferCont toAccount];
	
	Document* pDocument = Document::getInstance();
	if (!pDocument)
		return;
	
	Account *pFromAccount = NULL;
	pFromAccount = pDocument->getAccountPtr(nFromAccount);
	Account *pToAccount = NULL;
	pToAccount = pDocument->getAccountPtr(nToAccount);
	
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
	
	ValueFormatter* valueFormatter = [ValueFormatter sharedInterface];
	
	fixed amount = [valueFormatter fixedFromString:sAmount];
	
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
	
	if (!strCategory.empty() && !pDocument->doesCategoryExist(strCategory))
	{
		pDocument->addCategory(strCategory);
		[transactionsCategory addItemWithObjectValue:sCategory];
	}
	
	[self setDocumentModified:TRUE];
	
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
	
	// update the balances in the IndexBar
	
	fixed fromBalance = pFromAccount->getBalance(true);
	fixed toBalance = pToAccount->getBalance(true);
	
	NSString *sFromBalance = [[valueFormatter currencyStringFromFixed:fromBalance] retain];
	
	NSString *sFromAccountKey = [NSString stringWithFormat:@"a%d", nFromAccount];
	
	if (sFromAccountKey)
	{
		[indexBar updateAmount:sFromAccountKey amount:sFromBalance];		
	}
	
	NSString *sToBalance = [[valueFormatter currencyStringFromFixed:toBalance] retain];
	
	NSString *sToAccountKey = [NSString stringWithFormat:@"a%d", nToAccount];
	
	if (sToAccountKey)
	{
		[indexBar updateAmount:sToAccountKey amount:sToBalance];		
	}
	
	[indexBar reloadData];
}

- (void)setTransactionsViewType:(int)type
{
	m_showTransactionsViewType = type;
	[self buildTransactionsTree];
}

- (void)handleTransactionsSettingsUpdate:(NSNotification *)note
{
	[self buildTransactionsTree];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL action = [menuItem action];
	
	if (action == @selector(AddTransaction:))
	{
		if (!m_pAccount)
			return NO;
	}
	
	if (action == @selector(MoveUp:) || action == @selector(MoveDown:) || action == @selector(SplitTransaction:) ||
		action == @selector(DeleteTransaction:))
	{
		if (!m_pAccount)
			return NO;
		
		if (!m_SelectedTransaction)
			return NO;
	}
		
	return YES;
}

- (void)setDocumentModified:(BOOL)modified
{
	[fMainController setDocumentModified:modified];
}

@end
