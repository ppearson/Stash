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
	[self buildIndexTree];
	[self buildContentTree];
	[self updateUI];
	
	[window makeKeyAndOrderFront:self];
}

- (id)init
{
	self = [super init];
	if (self)
	{
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
	
	[contentView setFrameSize:[contentViewPlaceholder frame].size];
	[contentViewPlaceholder addSubview:contentView];
	
	m_aContentItems = [[NSMutableArray alloc] init];
	
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
	
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(TransactionSelectionDidChange:) name:NSOutlineViewSelectionDidChangeNotification object:transactionsTableView];
	
	Account acc;
	acc.setName("Main");
	
	m_Document.addAccount(acc);
	m_pAccount = m_Document.getAccountPtr(0);
	
/*	Transaction t0("Starting balance", "Desc", "Category", 2142.51, Date(13, 9, 2009));
	Transaction t1("Tax", "Council", "Tax", -86.00, Date(13, 9, 2009));
	Transaction t2("Food", "Sainsbury's", "Food", -13.44, Date(17, 9, 2009));
	Transaction t3("Pay", "Work", "Pay", 2470.0, Date(29, 9, 2009));
	
	t1.addSplit("Test1", "Test1", "T1", -30);
	t1.addSplit("Test2", "Test2", "T2", -21.44);
	
	m_pAccount->addTransaction(t0);
	m_pAccount->addTransaction(t1);
	m_pAccount->addTransaction(t2);
	m_pAccount->addTransaction(t3);
*/	
/*	for (int i = 0; i < 5000; i++)
	{
		Transaction t1("Tax", "Council", "Tax", -2466.00, Date(13, 9, 2009));
		Transaction t2("Food", "Sainsbury's", "Food", -13.44, Date(17, 9, 2009));
		Transaction t3("Pay", "Work", "Pay", 2470.0, Date(29, 9, 2009));
		
		m_pAccount->addTransaction(t1);
		m_pAccount->addTransaction(t2);
		m_pAccount->addTransaction(t3);
	}
*/	
	[transactionsTableView setDelegate:self];
	[transactionsTableView setAutoresizesOutlineColumn:NO];	
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
	
	[indexBar addSection:@"library" title:@"Library"];
	[indexBar addItem:@"library" key:@"payees" title:@"Payees" item:0 action:nil target:nil];
	[indexBar addItem:@"library" key:@"categories" title:@"Categories" item:0 action:nil target:nil];
	[indexBar addItem:@"library" key:@"schedules" title:@"Schedules" item:0 action:nil target:nil];
	
	m_pAccount = 0;
	
	m_SelectedTransaction = 0; 
	m_bEditing = false;
	
	[indexBar reloadData];
	
	[indexBar expandSection:@"accounts"];
	
	// automatically select the first account
	[indexBar selectItem:@"a0"];
}

- (void)accountSelected:(id)sender
{
	int nAccount = [sender getItemIndex];
		
	m_pAccount = m_Document.getAccountPtr(nAccount);			
	m_SelectedTransaction = 0;
	
	[Payee setStringValue:@""];
	[Description setStringValue:@""];
	[Category setStringValue:@""];
	[Amount setStringValue:@""];
	[Type selectItemAtIndex:0];
	[Reconciled setState:NSOffState];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:-1] byExtendingSelection:NO];
	
	[self buildContentTree];
	[self updateUI];	
}

- (void)buildContentTree
{
	[m_aContentItems removeAllObjects];
	
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
	
	for (; it != m_pAccount->end(); ++it, nTransaction++)
	{
		TransactionItem *newTransaction = [[TransactionItem alloc] init];
		
		std::string strTPayee = it->Payee();
		NSString *sTPayee = [[NSString alloc] initWithUTF8String:strTPayee.c_str()];
		
		std::string strTDescription = it->Description();
		NSString *sTDescription = [[NSString alloc] initWithUTF8String:strTDescription.c_str()];
		
		std::string strTCategory = it->Category();
		NSString *sTCategory = [[NSString alloc] initWithUTF8String:strTCategory.c_str()];
		
		std::string strTAmount = it->Amount();
		NSString *sTAmount = [[NSString alloc] initWithUTF8String:strTAmount.c_str()];
		
		std::string strTDate = it->Date1().FormattedDate(UK);
		NSString *sTDate = [[NSString alloc] initWithUTF8String:strTDate.c_str()];
		
		localBalance += it->Amount();
		
		std::string strTBalance = localBalance;
		NSString *sTBalance = [[NSString alloc] initWithUTF8String:strTBalance.c_str()];
		
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
				
				std::string strSAmount = split.Amount();
				NSString *sSAmount = [[NSString alloc] initWithUTF8String:strSAmount.c_str()];
				
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
		
		[m_aContentItems addObject:newTransaction];
		[sTPayee release];
		[sTDescription release];
		[sTCategory release];
		[sTAmount release];
		[sTDate release];
		[sTBalance release];
	}
	
	[transactionsTableView reloadData];
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
	
	std::string strAmount = newTransaction.Amount();
	NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
	
	std::string strDate = newTransaction.Date1().FormattedDate(UK);
	NSString *sDate = [[NSString alloc] initWithUTF8String:strDate.c_str()];
	
	fixed localBalance = m_pAccount->getBalance(true);
	
	std::string strBalance = localBalance;
	NSString *sBalance = [[NSString alloc] initWithUTF8String:strBalance.c_str()];
	
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
	
	[m_aContentItems addObject:newIndex];
	[newIndex release];
	
	[transactionsTableView reloadData];
	
	NSInteger row = [transactionsTableView rowForItem:newIndex];
	
	[transactionsTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	
	[window makeFirstResponder:Payee];
	
	m_UnsavedChanges = true;
	
	[self updateUI];
}

- (IBAction)Delete:(id)sender
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
				[m_aContentItems removeObjectAtIndex:nTransaction - m_nTransactionOffset];
			}
			else if (nSplit != -2)
			{
				TransactionItem *transactionItem = [m_aContentItems objectAtIndex:nTransaction - m_nTransactionOffset];
				Transaction &trans = m_pAccount->getTransaction(nTransaction);
				
				trans.deleteSplit(nSplit);
				[transactionItem deleteChild:nSplit];
			}
			
			row = [rows indexLessThanIndex:row];			
		}
		
		// TODO: for multiple selection this won't work, as indexes will get out of sync
		//		 also, when removing items, the balance value of other objects will be wrong
		
//		[self buildContentTree];
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
	
	std::string strAmount = splitValue;
	NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
	
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
	
	m_UnsavedChanges = true;
}

- (void)TransactionSelectionDidChange:(NSNotification *)notification
{
	if (!m_pAccount)
		return;
	
	NSIndexSet *rows = [transactionsTableView selectedRowIndexes];
	
	if ([rows count] == 1)
	{
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
			NSDate *datetemp = convertToNSDate(&date1);
			
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
	std::string strDate = date1.FormattedDate(UK);
	NSString *sDate = [[NSString alloc] initWithUTF8String:strDate.c_str()];
	
	std::string strPayee = [[Payee stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strDesc = [[Description stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strCategory = [[Category stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strAmount = [[Amount stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	
	fixed fAmount = strAmount;
	NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
	
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
			
			fixed splitValue = trans->getSplitTotal();
			
			fixed diff = transValue -= splitValue;
			
			// Then add a new dummy value if needed
			
			if (!diff.IsZero())
			{
				TransactionItem *transIndex = [m_aContentItems objectAtIndex:nTrans - m_nTransactionOffset];
				
				TransactionItem *newSplit = [[TransactionItem alloc] init];
				
				std::string strAmount = diff;
				NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
				
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
	
	m_UnsavedChanges = true;
	
	[self updateUI];
}

- (IBAction)showLast100Transactions:(id)sender
{
	nShowTransactionsType = LAST_100;
	[self buildContentTree];
}

- (IBAction)showAllTransactionsThisYear:(id)sender
{
	nShowTransactionsType = ALL_THIS_YEAR;
	[self buildContentTree];
}

- (IBAction)showAllTransactions:(id)sender
{
	nShowTransactionsType = ALL;
	[self buildContentTree];
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if (item == nil)
	{
		return [m_aContentItems objectAtIndex:index];
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
			return [m_aContentItems count];
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

NSDate * convertToNSDate(Date *date)
{
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
    
    NSDateComponents *dateComponents = [[[NSDateComponents alloc] init] autorelease];
    [dateComponents setYear:date->Year()];
    [dateComponents setMonth:date->Month()];
    [dateComponents setDay:date->Day()];
    
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
		[self buildContentTree];
		[self refreshLibraryItems];
		
		[self updateUI];
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
		[self buildContentTree];	
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

- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    SEL action = [item action];
    if (action == @selector(toggleVisibleControllerView:))
	{
		
		
	}
	
	return YES;
}

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
	[self buildContentTree];
	[self refreshLibraryItems];
	
	[self updateUI];

	return YES;
}

@end
