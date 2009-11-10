//
//  StashAppDelegate.m
//  Stash
//
//  Created by Peter Pearson on 24/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "StashAppDelegate.h"
#import "IndexItem.h"
#include "storage.h"

@implementation StashAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{	
	m_HasFinishedLoading = true;
	
	if (m_sPendingOpenFile != nil)
	{
		[self application:nil openFile:m_sPendingOpenFile];
	}
	else
	{
		[self buildIndexTree];
	
		[window makeKeyAndOrderFront:self];
	}
}

- (id)init
{
	self = [super init];
	if (self)
	{
		m_HasFinishedLoading = false;
		m_sPendingOpenFile = nil;
		
		[NSApp setDelegate: self];
	}
	
	return self;
}

- (void)awakeFromNib
{
	m_bEditing = false;	
	m_SelectedTransaction = 0;
	
	nShowTransactionsType = LAST_100;
	
	[window setDelegate:self];
	
	[indexView setFrameSize:[indexViewPlaceholder frame].size];
	[indexViewPlaceholder addSubview:indexView];
	
	[vTransactionsView setFrameSize:[contentViewPlaceholder frame].size];
	
	[contentViewPlaceholder addSubview:vTransactionsView];
	contentView = vTransactionsView;
	
	m_aTransactionItems = [[NSMutableArray alloc] init];
	m_aPayeeItems = [[NSMutableArray alloc] init];
	m_aCategoryItems = [[NSMutableArray alloc] init];
	
	NSDate *date1 = [NSDate date];
	[DateCntl setDateValue:date1];
	
	[Type removeAllItems];
	
	[Type addItemWithTitle:@"None"];
	[Type addItemWithTitle:@"Deposit"];
	[Type addItemWithTitle:@"Withdrawal"];
	[Type addItemWithTitle:@"Transfer"];
	[Type addItemWithTitle:@"Standing Order"];
	[Type addItemWithTitle:@"Direct Debit"];
	[Type addItemWithTitle:@"Point Of Sale"];
	[Type addItemWithTitle:@"Charge"];
	[Type addItemWithTitle:@"ATM"];
	
	[Type selectItemAtIndex:0];
	
	[addTransaction setToolTip:@"Add Transaction"];
	[deleteTransaction setToolTip:@"Delete Transaction"];
	[splitTransaction setToolTip:@"Split Transaction"];
	
	[deleteTransaction setEnabled:NO];
	[splitTransaction setEnabled:NO];
	[moveUp setEnabled:NO];
	[moveDown setEnabled:NO];
	
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(TransactionSelectionDidChange:) name:NSOutlineViewSelectionDidChangeNotification object:transactionsTableView];
	
	Account acc;
	acc.setName("Main");
	
	m_Document.addAccount(acc);
	m_pAccount = m_Document.getAccountPtr(0);
	
	[transactionsTableView setDelegate:self];
	[transactionsTableView setAutoresizesOutlineColumn:NO];
	
	[payeesTableView setDelegate:self];
	
	[categoriesTableView setDelegate:self];
}

- (void)buildIndexTree
{
	[indexBar clearAllItems];
	
	[indexBar addSection:@"accounts" title:@"ACCOUNTS"];
	
	std::vector<Account>::iterator it = m_Document.AccountBegin();
	
	int nAccount = 0;
	
	for (; it != m_Document.AccountEnd(); ++it, nAccount++)
	{
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		
		NSString *sAccountKey = [NSString stringWithFormat:@"a%d", nAccount];

		[indexBar addItem:@"accounts" key:sAccountKey title:sName item:nAccount action:@selector(accountSelected:) target:self];
		
		[sName release];
	}
	
	[indexBar addSection:@"manage" title:@"MANAGE"];
	[indexBar addItem:@"manage" key:@"payees" title:@"Payees" item:0 action:@selector(payeesSelected:) target:self];
	[indexBar addItem:@"manage" key:@"categories" title:@"Categories" item:0 action:@selector(categoriesSelected:) target:self];
	[indexBar addItem:@"manage" key:@"scheduled" title:@"Scheduled" item:0 action:nil target:nil];
	
	m_pAccount = 0;
	
	m_SelectedTransaction = 0; 
	m_bEditing = false;
	
	[indexBar reloadData];
	
	[indexBar expandSection:@"accounts"];
	[indexBar expandSection:@"manage"];
	
	// automatically select the first account
	[indexBar selectItem:@"a0"];
}

- (void)accountSelected:(id)sender
{
	int nAccount = [sender getItemIndex];
		
	m_pAccount = m_Document.getAccountPtr(nAccount);			
	m_SelectedTransaction = 0;
	
	[vTransactionsView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vTransactionsView];
	contentView = vTransactionsView;
	
	[Payee setStringValue:@""];
	[Description setStringValue:@""];
	[Category setStringValue:@""];
	[Amount setStringValue:@""];
	[Type selectItemAtIndex:0];
	[Reconciled setState:NSOffState];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:-1] byExtendingSelection:NO];
	
	[self buildTransactionsTree];
	[self updateUI];	
}

- (void)payeesSelected:(id)sender
{
	[vPayeesView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vPayeesView];
	contentView = vPayeesView;
	
	[self buildPayeesList];
}

- (void)categoriesSelected:(id)sender
{
	[vCategoriesView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder replaceSubview:contentView with:vCategoriesView];
	contentView = vCategoriesView;
	
	[self buildCategoriesList];
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
		return;
	
	std::vector<Transaction>::iterator it = m_pAccount->begin();
	int nTransaction = 0;
	m_nTransactionOffset = 0;
	
	if (nShowTransactionsType == LAST_100)
	{
		int nTransactionsToShow = 100;
		if (m_pAccount->getTransactionCount() > nTransactionsToShow)
		{
			// calculate balance without outputting it
			std::vector<Transaction>::iterator stopIt = m_pAccount->end() - nTransactionsToShow;
			for (; it != stopIt; ++it, nTransaction++)
			{
				localBalance += (*it).Amount();
			}
			
			m_nTransactionOffset = nTransaction;
		}
	}
	else if (nShowTransactionsType == ALL_THIS_YEAR)
	{
		Date dateNow;
		dateNow.Now();
		
		Date dateComp(1, 1, dateNow.Year());
		
		std::vector<Transaction>::iterator itTemp = m_pAccount->begin();
		
		for (; itTemp != m_pAccount->end(); ++itTemp)
		{
			if ((*itTemp).Date1() >= dateComp)
				break;
			
			localBalance += (*itTemp).Amount();
			nTransaction++;
		}
		
		it = itTemp;
		
		m_nTransactionOffset = nTransaction;
	}
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	for (; it != m_pAccount->end(); ++it, nTransaction++)
	{
		TransactionItem *newTransaction = [[TransactionItem alloc] init];
		
		std::string strTPayee = it->Payee();
		NSString *sTPayee = [[NSString alloc] initWithUTF8String:strTPayee.c_str()];
		
		std::string strTDescription = it->Description();
		NSString *sTDescription = [[NSString alloc] initWithUTF8String:strTDescription.c_str()];
		
		std::string strTCategory = it->Category();
		NSString *sTCategory = [[NSString alloc] initWithUTF8String:strTCategory.c_str()];
		
		NSNumber *nTAmount = [NSNumber numberWithDouble:it->Amount().ToDouble()];
		
		NSString *sTAmount = [[numberFormatter stringFromNumber:nTAmount] retain];
		
		NSDate *date = convertToNSDate(const_cast<Date&>(it->Date2()));
		NSString *sTDate = [[dateFormatter stringFromDate:date] retain];
		
		localBalance += it->Amount();
		
		NSNumber *nTBalance = [NSNumber numberWithDouble:localBalance.ToDouble()];
		NSString *sTBalance = [[numberFormatter stringFromNumber:nTBalance] retain];
		
		[newTransaction setTransaction:nTransaction];
		
		int recon = it->isReconciled();
		
		[newTransaction setIntValue:recon forKey:@"Reconciled"];
		[newTransaction setValue:sTDate forKey:@"Date"];
		[newTransaction setValue:sTPayee forKey:@"Payee"];
		[newTransaction setValue:sTDescription forKey:@"Description"];
		[newTransaction setValue:sTCategory forKey:@"Category"];
		[newTransaction setValue:sTAmount forKey:@"Amount"];
		[newTransaction setValue:sTBalance forKey:@"Balance"];
		
		[newTransaction setIntValue:nTransaction forKey:@"Transaction"];
		
		if (it->Split())
		{
			int nSplits = it->getSplitCount();
			
			fixed splitValue = it->Amount();
			
			for (int i = 0; i < nSplits; i++)
			{
				SplitTransaction & split = it->getSplit(i);
				
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				std::string strSPayee = split.Payee();
				NSString *sSPayee = [[NSString alloc] initWithUTF8String:strSPayee.c_str()];
				
				std::string strSDescription = split.Description();
				NSString *sSDescription = [[NSString alloc] initWithUTF8String:strSDescription.c_str()];
				
				std::string strSCategory = split.Category();
				NSString *sSCategory = [[NSString alloc] initWithUTF8String:strSCategory.c_str()];
				
				NSNumber *nSAmount = [NSNumber numberWithDouble:splitValue.ToDouble()];
				NSString *sSAmount = [[numberFormatter stringFromNumber:nSAmount] retain];
				
				[newSplit setValue:sSPayee forKey:@"Payee"];
				[newSplit setValue:sSDescription forKey:@"Description"];
				[newSplit setValue:sSCategory forKey:@"Category"];
				[newSplit setValue:sSAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:i];
				[newSplit setIntValue:i forKey:@"Split"];
				
				splitValue -= split.Amount();
				
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
				
				std::string strSAmount = splitValue;
				NSString *sSAmount = [[NSString alloc] initWithUTF8String:strSAmount.c_str()];
				
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

- (void)refreshLibraryItems
{
	[Payee removeAllItems];
	
	std::set<std::string>::iterator it = m_Document.PayeeBegin();
	
	for (; it != m_Document.PayeeEnd(); ++it)
	{
		NSString *sPayee = [[NSString alloc] initWithUTF8String:(*it).c_str()];
		
		[Payee addItemWithObjectValue:sPayee];
	}
	
	[Category removeAllItems];
	
	std::set<std::string>::iterator itCat = m_Document.CategoryBegin();
	
	for (; itCat != m_Document.CategoryEnd(); ++itCat)
	{
		NSString *sCategory = [[NSString alloc] initWithUTF8String:(*itCat).c_str()];
		
		[Category addItemWithObjectValue:sCategory];
	}
}

- (void)updateUI
{
	if (!m_pAccount)
		return;
	
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
	
	std::string strName = [sAccountName cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strStartingBalance = [sStartingBalance cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strInstitution = [sInstitution cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strNumber = [sNumber cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strNote = [sNote cStringUsingEncoding:NSASCIIStringEncoding];
	
	fixed startingBalance(strStartingBalance, UKFormat);
	
	Account newAccount;
	newAccount.setName(strName);
	newAccount.setType(eType);
	newAccount.setInstitution(strInstitution);
	newAccount.setNumber(strNumber);
	newAccount.setNote(strNote);
	
	Transaction newTransaction("Starting balance", "", "", startingBalance, -1);
	newTransaction.setReconciled(true);
	
	newAccount.addTransaction(newTransaction);
	
	int nAccountNum = m_Document.addAccount(newAccount);
	
	NSString *sAccountKey = [NSString stringWithFormat:@"a@s", nAccountNum];
	
	[indexBar addItem:@"accounts" key:sAccountKey title:sAccountName item:nAccountNum action:@selector(accountSelected:) target:self];
	
	[addAccountController release];
	
	[indexBar reloadData];
}

- (IBAction)AddTransaction:(id)sender
{
	if (!m_pAccount)
		return;
	
	Transaction newTransaction("", "", "", 0.0, -1);
	
	m_pAccount->addTransaction(newTransaction);
	
	[Payee setStringValue:@""];
	[Description setStringValue:@""];
	[Category setStringValue:@""];
	[Amount setStringValue:@""];
	[Type selectItemAtIndex:0];
	[Reconciled setState:NSOffState];
	
	TransactionItem *newIndex = [[TransactionItem alloc] init];
	
	std::string strPayee = newTransaction.Payee();
	NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
	
	std::string strDescription = newTransaction.Description();
	NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
	
	std::string strCategory = newTransaction.Category();
	NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	NSNumber *nAmount = [NSNumber numberWithDouble:newTransaction.Amount().ToDouble()];
	NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
	
	NSDate *date = convertToNSDate(const_cast<Date&>(newTransaction.Date2()));
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	NSString *sDate = [[dateFormatter stringFromDate:date] retain];
	
	[dateFormatter release];
	
	fixed localBalance = m_pAccount->getBalance(true);
	
	NSNumber *nBalance = [NSNumber numberWithDouble:localBalance.ToDouble()];
	NSString *sBalance = [[numberFormatter stringFromNumber:nBalance] retain];
	
	[numberFormatter release];
	
	int nTransaction = m_pAccount->getTransactionCount() - 1;
	
	[newIndex setTransaction:nTransaction];
	
	int recon = newTransaction.isReconciled();
	
	[newIndex setIntValue:recon forKey:@"Reconciled"];
	[newIndex setValue:sDate forKey:@"Date"];
	[newIndex setValue:sPayee forKey:@"Payee"];
	[newIndex setValue:sDescription forKey:@"Description"];
	[newIndex setValue:sCategory forKey:@"Category"];
	[newIndex setValue:sAmount forKey:@"Amount"];
	[newIndex setValue:sBalance forKey:@"Balance"];
	
	[newIndex setIntValue:nTransaction forKey:@"Transaction"];
	
	[m_aTransactionItems addObject:newIndex];
	[newIndex release];
	
	[transactionsTableView reloadData];
	
	NSInteger row = [transactionsTableView rowForItem:newIndex];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	
	[window makeFirstResponder:Payee];
	
	m_UnsavedChanges = true;
	
	[self updateUI];
}

- (IBAction)DeleteTransaction:(id)sender
{
	NSIndexSet *rows = [transactionsTableView selectedRowIndexes];
	
	if ([rows count] > 0)
	{
		NSInteger row = [rows lastIndex];
		
		while (row != NSNotFound)
		{
			TransactionItem *item = [transactionsTableView itemAtRow:row];
			
			int nSplit = [item intKeyValue:@"Split"];
			int nTransaction = [item intKeyValue:@"Transaction"];
			
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
			
			row = [rows indexLessThanIndex:row];			
		}
		
		// TODO: for multiple selection this won't work, as indexes will get out of sync
		//		 also, when removing items, the balance value of other objects will be wrong
		
		[transactionsTableView reloadData];
		
		m_UnsavedChanges = true;
		
		[self updateUI];
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
	
	fixed splitValue = trans.Amount();
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
	
	[window makeFirstResponder:Payee];
	
	[numberFormatter release];
	
	m_UnsavedChanges = true;
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
		
		int nSplit = [thisItem splitTransaction];
		
		if (nSplit >= 0)
		{
			return;
		}
		
		// Swap them over
		[self SwapTransactions:row to:row - 1];
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
		
		int nSplit = [thisItem splitTransaction];
		
		if (nSplit >= 0)
		{
			return;
		}
		
		// Swap them over
		[self SwapTransactions:row to:row + 1];
	}
}

- (void)SwapTransactions:(int)from to:(int)to
{
	if (from < 0 || to < 0)
		return;
	
	int maxTransactionIndex = [m_aTransactionItems count] - 1;
	
	if (from > maxTransactionIndex || to > maxTransactionIndex)
		return;
	
	int nRealFrom = from + m_nTransactionOffset;
	int nRealTo = to + m_nTransactionOffset;
	
	m_pAccount->swapTransactions(nRealFrom, nRealTo);
	[m_aTransactionItems exchangeObjectAtIndex:from withObjectAtIndex:to];
	
	// TransactionItem values are still pointing in the wrong place, so fix them....
	
	TransactionItem *fromItem = [transactionsTableView itemAtRow:from];
	TransactionItem *toItem = [transactionsTableView itemAtRow:to];
	
	[fromItem setTransaction:nRealTo];
	[fromItem setIntValue:nRealTo forKey:@"Transaction"];
	
	[toItem setTransaction:nRealFrom];
	[toItem setIntValue:nRealFrom forKey:@"Transaction"];
		
	[transactionsTableView reloadData];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:to] byExtendingSelection:NO];
	
	m_UnsavedChanges = true;
}

- (void)RefreshView:(id)sender
{
	[self buildTransactionsTree];
	
	[transactionsTableView reloadData];	
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
		
		if (trans && !split && nSplit != -2) // A normal transaction
		{
			[splitTransaction setEnabled:YES];
			[moveUp setEnabled:YES];
			[moveDown setEnabled:YES];
			
			m_bEditing = true;			
			
			std::string strPayee = trans->Payee();
			NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
			
			std::string strDescription = trans->Description();
			NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
			
			std::string strCategory = trans->Category();
			NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
			
			std::string strAmount = trans->Amount();
			NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
			
			TransactionType eType = trans->Type();
			
			Date date1 = trans->Date1();
			NSDate *datetemp = convertToNSDate(date1);
			
			[Reconciled setEnabled:YES];
			[Type setEnabled:YES];
			
			bool bReconciled = trans->isReconciled();
			
			if (bReconciled)
			{
				[Reconciled setState:NSOnState];
			}
			else
			{
				[Reconciled setState:NSOffState];
			}
			
			[Payee setStringValue:sPayee];
			[Description setStringValue:sDescription];
			[Category setStringValue:sCategory];
			[Amount setStringValue:sAmount];
			[DateCntl setDateValue:datetemp];
			[Type selectItemAtIndex:eType];
			
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
			
			std::string strPayee = split->Payee();
			NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
			
			std::string strDescription = split->Description();
			NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
			
			std::string strCategory = split->Category();
			NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
			
			std::string strAmount = split->Amount();
			NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
			
			[Payee setStringValue:sPayee];
			[Description setStringValue:sDescription];
			[Category setStringValue:sCategory];
			[Amount setStringValue:sAmount];
			[Type selectItemAtIndex:0];
			
			[Reconciled setEnabled:NO];
			[Type setEnabled:NO];
			
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
			
			[Payee setStringValue:sPayee];
			[Description setStringValue:sDescription];
			[Category setStringValue:@""];
			[Amount setStringValue:sAmount];
			[Type selectItemAtIndex:0];
			
			[Reconciled setEnabled:NO];
			[Type setEnabled:NO];
		}		
	}
	else
	{
		[deleteTransaction setEnabled:NO];
		[splitTransaction setEnabled:NO];
		[moveUp setEnabled:NO];
		[moveDown setEnabled:NO];
		
		m_SelectedTransaction = 0; 
		m_bEditing = false;		
	}
}

- (void)updateTransaction:(id)sender
{
	if (!m_bEditing || !m_SelectedTransaction)
		return;
	
	NSDate *ndate1 = [DateCntl dateValue];
	NSCalendarDate *CalDate = [ndate1 dateWithCalendarFormat:0 timeZone:0];
	
	int nYear = [CalDate yearOfCommonEra];
	int nMonth = [CalDate monthOfYear];
	int nDay = [CalDate dayOfMonth];
	
	Date date1(nDay, nMonth, nYear);
	
	NSDate *date = convertToNSDate(const_cast<Date&>(date1));
	
	[NSDateFormatter setDefaultFormatterBehavior:NSDateFormatterBehavior10_4];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateStyle:NSDateFormatterShortStyle];
	[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
	NSString *sDate = [[dateFormatter stringFromDate:date] retain];
	
	[dateFormatter release];
	
	std::string strPayee = [[Payee stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strDesc = [[Description stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strCategory = [[Category stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strAmount = [[Amount stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	
	fixed fAmount = strAmount;
	NSNumber *nAmount = [NSNumber numberWithDouble:fAmount.ToDouble()];
	NSString *sAmount = [[numberFormatter stringFromNumber:nAmount] retain];
	
	int nType = [Type indexOfSelectedItem];
	TransactionType eType = static_cast<TransactionType>(nType);
	
	bool bReconciled = false;
	
	if ([Reconciled state] == NSOnState)
		bReconciled = true;
	
	int nTrans = [m_SelectedTransaction transaction];
	int nSplit = [m_SelectedTransaction splitTransaction];
	
	Transaction *trans = NULL;
	SplitTransaction *split = NULL;
	
	if (nTrans >= 0)
		trans = &m_pAccount->getTransaction(nTrans);
	
	if (nSplit >= 0)
		split = &trans->getSplit(nSplit);
	
	if (trans && !split && nSplit != -2)
	{
		trans->setDate(date1);
		trans->setPayee(strPayee);
		trans->setDescription(strDesc);
		trans->setCategory(strCategory);
		trans->setAmount(fAmount);
		trans->setType(eType);
		trans->setReconciled(bReconciled);
		
		[m_SelectedTransaction setValue:sDate forKey:@"Date"];
		[m_SelectedTransaction setValue:[Payee stringValue] forKey:@"Payee"];
		[m_SelectedTransaction setValue:[Description stringValue] forKey:@"Description"];
		[m_SelectedTransaction setValue:[Category stringValue] forKey:@"Category"];
		[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
		[m_SelectedTransaction setIntValue:bReconciled forKey:@"Reconciled"];
		
		if (!strPayee.empty() && !m_Document.doesPayeeExist(strPayee))
		{
			m_Document.addPayee(strPayee);
			[Payee addItemWithObjectValue:[Payee stringValue]];
		}
		
		if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
		{
			m_Document.addCategory(strCategory);
			[Category addItemWithObjectValue:[Category stringValue]];
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
			
			[m_SelectedTransaction setValue:[Payee stringValue] forKey:@"Payee"];
			[m_SelectedTransaction setValue:[Description stringValue] forKey:@"Description"];
			[m_SelectedTransaction setValue:[Category stringValue] forKey:@"Category"];
			[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
			[m_SelectedTransaction setIntValue:bReconciled forKey:@"Reconciled"];
			
			if (!strPayee.empty() && !m_Document.doesPayeeExist(strPayee))
			{
				m_Document.addPayee(strPayee);
				[Payee addItemWithObjectValue:[Payee stringValue]];
			}
			
			if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
			{
				m_Document.addCategory(strCategory);
				[Category addItemWithObjectValue:[Category stringValue]];
			}
			
			[transactionsTableView reloadData];
		}
		else if (nSplit == -2) // Dummy value, so convert to a real split
		{
			fixed transValue = trans->Amount();			
			
			trans->addSplit(strDesc, strPayee, strCategory, fAmount);
			
			int nSplitsNumber = trans->getSplitCount() - 1;
			[m_SelectedTransaction setSplitTransaction:nSplitsNumber];
			[m_SelectedTransaction setIntValue:nSplitsNumber forKey:@"Split"];
			[m_SelectedTransaction setValue:[Payee stringValue] forKey:@"Payee"];
			[m_SelectedTransaction setValue:[Description stringValue] forKey:@"Description"];
			[m_SelectedTransaction setValue:[Category stringValue] forKey:@"Category"];
			[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
			
			if (!strPayee.empty() && !m_Document.doesPayeeExist(strPayee))
			{
				m_Document.addPayee(strPayee);
				[Payee addItemWithObjectValue:[Payee stringValue]];
			}
			
			if (!strCategory.empty() && !m_Document.doesCategoryExist(strCategory))
			{
				m_Document.addCategory(strCategory);
				[Category addItemWithObjectValue:[Category stringValue]];
			}
			
			fixed splitValue = trans->getSplitTotal();
			
			fixed diff = transValue -= splitValue;
			
			// Then add a new dummy value if needed
			
			if (!diff.IsZero())
			{
				TransactionItem *transIndex = [m_aTransactionItems objectAtIndex:nTrans - m_nTransactionOffset];
				
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				std::string strSAmount = diff;
				NSString *sAmount = [[NSString alloc] initWithUTF8String:strSAmount.c_str()];
				
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:@"Split Value" forKey:@"Description"];
				[newSplit setValue:sAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTrans];
				[newSplit setIntValue:nTrans forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:-2];
				[newSplit setIntValue:-2 forKey:@"Split"];
				
				[transIndex addChild:newSplit];
				
				[transactionsTableView reloadData];
				
				NSInteger row = [transactionsTableView rowForItem:newSplit];
				
				[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
				
				[window makeFirstResponder:Payee];
			}
			else
			{
				[transactionsTableView reloadData];
			}
		}		
	}
	
	[numberFormatter release];
	
	m_UnsavedChanges = true;
	
	[self updateUI];
}

- (IBAction)showLast100Transactions:(id)sender
{
	nShowTransactionsType = LAST_100;
	[self buildTransactionsTree];
}

- (IBAction)showAllTransactionsThisYear:(id)sender
{
	nShowTransactionsType = ALL_THIS_YEAR;
	[self buildTransactionsTree];
}

- (IBAction)showAllTransactions:(id)sender
{
	nShowTransactionsType = ALL;
	[self buildTransactionsTree];
}

- (IBAction)DeletePayee:(id)sender
{
	NSInteger row = [payeesTableView selectedRow];
	
	if (row >= 0)
	{
		NSString *sPayee = [m_aPayeeItems objectAtIndex:row];
		
		std::string strPayee = [sPayee cStringUsingEncoding:NSASCIIStringEncoding];
		
		[m_aPayeeItems removeObjectAtIndex:row];
		
		m_Document.deletePayee(strPayee);
		
		[payeesTableView reloadData];
		
		m_UnsavedChanges = true;
	}
}

- (IBAction)DeleteCategory:(id)sender
{
	NSInteger row = [categoriesTableView selectedRow];
	
	if (row >= 0)
	{
		NSString *sCategory = [m_aCategoryItems objectAtIndex:row];
		
		std::string strCategory = [sCategory cStringUsingEncoding:NSASCIIStringEncoding];
		
		[m_aCategoryItems removeObjectAtIndex:row];
		
		m_Document.deleteCategory(strCategory);
		
		[categoriesTableView reloadData];
		
		m_UnsavedChanges = true;
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
	
	if ([identifier caseInsensitiveCompare:@"reconciled"] == NSOrderedSame)
	{
		int temp = [item intKeyValue:identifier];
		return [NSNumber numberWithInt:temp];
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
			//		int nSplit = [m_SelectedItem splitTransaction];
			
			Transaction *trans = NULL;
			//		SplitTransaction *split = NULL;
			
			if (nTrans >= 0)
				trans = &m_pAccount->getTransaction(nTrans);
			
			if ([identifier isEqualToString:@"Reconciled"])
			{
				if ([object boolValue] == YES)
				{
					trans->setReconciled(true);
				}
				else
				{
					trans->setReconciled(false);
				}
				
				m_UnsavedChanges = true;
				
				[self updateUI];
			}
		}
	}
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	BOOL result = YES;
	
	NSString *identifier = [tableColumn identifier];
	
	if ([identifier caseInsensitiveCompare:@"balance"] == 0)
		result = NO;
	
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
	
	if ([[tableColumn identifier] isEqualToString:@"Reconciled"])
	{
		if (enableCheck == YES)
		{
			[cell setImagePosition: NSImageOnly];
		}
		else
		{
			[cell setImagePosition: NSNoImage];
		}
	}
	else
		[cell setTextColor:fontColor];
}

// Transactions OutlineView End

// Payees/Categories TableView Start

- (id)tableView:(NSTableView *) aTableView objectValueForTableColumn:(NSTableColumn *) aTableColumn row:(NSInteger) rowIndex
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
		
	return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	if (aTableView == payeesTableView)
		return [m_aPayeeItems count];
	else if (aTableView == categoriesTableView)
		return [m_aCategoryItems count];
	
	return 0;
}

// Payees/Categories TableView End

NSDate * convertToNSDate(Date &date)
{
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
    
    NSDateComponents *dateComponents = [[[NSDateComponents alloc] init] autorelease];
    [dateComponents setYear:date.Year()];
    [dateComponents setMonth:date.Month()];
    [dateComponents setDay:date.Day()];
    
    [dateComponents setHour:0];
    [dateComponents setMinute:0];
    [dateComponents setSecond:0];
	
	NSDate *nsDate = [gregorian dateFromComponents:dateComponents];
	[gregorian release];
	
    return nsDate;
}

- (IBAction)OpenFile:(id)sender
{
	if (m_UnsavedChanges)
    {
		int choice = NSAlertDefaultReturn;
		
		NSString *message = @"Do you want to save the changes you made in this document?";
		
		choice = NSRunAlertPanel(message, @"Your changes will be lost if you don't save them.", @"Save", @"Don't Save", @"Cancel");
		
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
	[oPanel setTitle: @"Open Document"];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"stash", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
		fileToOpen = [oPanel filename];
		strFile = [fileToOpen cStringUsingEncoding: NSASCIIStringEncoding];
		
		if ([self OpenFileAt:strFile] == false)
		{
			NSAlert *alert = [[NSAlert alloc] init];
			[alert setMessageText:[NSString stringWithFormat:@"Could not open file: \"%@\".", fileToOpen]];
			[alert setInformativeText: @"There was a problem opening the Document file."];
			[alert setAlertStyle: NSWarningAlertStyle];
			[alert addButtonWithTitle: @"OK"];
			
			[alert runModal];
			[alert release];
			return;
		}
		
		m_DocumentFile = strFile;
		m_UnsavedChanges = false;
		
		m_SelectedTransaction = 0;
		m_pAccount = 0;
		
		[self buildIndexTree];
		[self refreshLibraryItems];
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
	
	[sPanel setTitle: @"Save Document"];
	[sPanel setRequiredFileType:@"stash"];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"stash", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
		fileToSave = [sPanel filename];
		strFile = [fileToSave cStringUsingEncoding: NSASCIIStringEncoding];
		
		if ([self SaveFileTo:strFile] == false)
		{
			NSAlert *alert = [[NSAlert alloc] init];
			[alert setMessageText:[NSString stringWithFormat:@"Could not save to file: \"%@\".", fileToSave]];
			[alert setInformativeText: @"There was a problem saving the Document to the file."];
			[alert setAlertStyle: NSWarningAlertStyle];
			[alert addButtonWithTitle: @"OK"];
			
			[alert runModal];
			[alert release];
		}
		else
		{
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
	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::binary);
	
	if (!fileStream)
	{
		return false;
	}
	
	m_Document.Store(fileStream);
	
	fileStream.close();
	
	return true;
}

- (IBAction)ImportQIF:(id)sender
{
	NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	NSString *fileToOpen;
	std::string strFile = "";
	[oPanel setAllowsMultipleSelection: NO];
	[oPanel setResolvesAliases: YES];
	[oPanel setTitle: @"Import QIF file"];
	[oPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"qif", nil]];
	
	if ([oPanel runModal] == NSOKButton)
	{
		fileToOpen = [oPanel filename];
		strFile = [fileToOpen cStringUsingEncoding: NSASCIIStringEncoding];
		
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
	
	NSString *sFile = [importQIFController file];
		
	std::string strFile = [sFile cStringUsingEncoding: NSASCIIStringEncoding];
	
	[importQIFController release];
	
	if (importQIFFileToAccount(m_pAccount, strFile, dateFormat, cSeparator))
		[self buildTransactionsTree];	
}

- (IBAction)ExportQIF:(id)sender
{
	NSSavePanel *sPanel = [NSSavePanel savePanel];
	NSString *fileToSave;
	std::string strFile = "";
	
	[sPanel setTitle: @"Export to QIF file"];
	[sPanel setRequiredFileType:@"qif"];
	[sPanel setAllowedFileTypes:[NSArray arrayWithObjects: @"qif", nil]];
	
	if ([sPanel runModal] == NSOKButton)
	{
		fileToSave = [sPanel filename];
		strFile = [fileToSave cStringUsingEncoding: NSASCIIStringEncoding];
		
		exportAccountToQIFFile(m_pAccount, strFile, UK);
	}	
}

/*- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    SEL action = [item action];
    if (action == @selector(Delete:))
	{
		
		
	}
	
	return YES;
}
*/
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	if (m_UnsavedChanges)
    {
		int choice = NSAlertDefaultReturn;
		
		NSString *message = @"Do you want to save the changes you made in this document?";
		
		choice = NSRunAlertPanel(message, @"Your changes will be lost if you don't save them.", @"Save", @"Don't Save", @"Cancel");
		
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
		
        NSString * message = @"You have unsaved changes in this document. Are you sure you want to open another document?";
		
		choice = NSRunAlertPanel(@"Replace current document?", message, @"Replace", @"Cancel", nil);
		
		if (choice != NSAlertDefaultReturn)
			return NO;
    }
	
	std::string strFile = [filename cStringUsingEncoding: NSASCIIStringEncoding];
	
	if ([self OpenFileAt:strFile] == false)
	{
		return NO;
	}
	
	m_DocumentFile = strFile;
	m_UnsavedChanges = false;
	
	[self buildIndexTree];
	[self refreshLibraryItems];

	return YES;
}

@end
