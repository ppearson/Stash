//
//  StashAppDelegate.m
//  Stash
//
//  Created by Peter Pearson on 24/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "StashAppDelegate.h"

@implementation StashAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
}

- (id)init
{
	self = [super init];
	if (self)
	{
		[NSApp setDelegate: self];
		NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
		[nc addObserver:self selector:@selector(TransactionSelectionDidChange:) name:NSOutlineViewSelectionDidChangeNotification object:contentView];
	}
	
	return self;
}

- (void) awakeFromNib
{
	m_bEditing = false;	
	m_SelectedTransaction = 0;
	
	m_aContentItems = [[NSMutableArray alloc] init];
	m_aIndexItems = [[NSMutableArray alloc] init];
	
	NSDate *date1 = [NSDate date];
	[DateCntl setDateValue:date1];
	
	Account acc;
	acc.setName("Main");
	
	m_aAccounts.push_back(acc);
	m_pAccount = &m_aAccounts[0];
	
	Transaction t0("Starting balance", "", "", 2142.51, Date(13, 9, 2009));
	Transaction t1("Tax", "Council", "Tax", -86.00, Date(13, 9, 2009));
	Transaction t2("Food", "Sainsbury's", "Food", -13.44, Date(17, 9, 2009));
	Transaction t3("Pay", "Work", "Pay", 2470.0, Date(29, 9, 2009));
	
	t1.addSplit("Test1", "Test1", "T1", -30);
	t1.addSplit("Test2", "Test2", "T2", -21.44);
	
	m_pAccount->addTransaction(t0);
	m_pAccount->addTransaction(t1);
	m_pAccount->addTransaction(t2);
	m_pAccount->addTransaction(t3);
	
	for (int i = 0; i < 50; i++)
	{
		Transaction t1("Tax", "Council", "Tax", -86.00, Date(13, 9, 2009));
		Transaction t2("Food", "Sainsbury's", "Food", -13.44, Date(17, 9, 2009));
		Transaction t3("Pay", "Work", "Pay", 2470.0, Date(29, 9, 2009));
		
		m_pAccount->addTransaction(t1);
		m_pAccount->addTransaction(t2);
		m_pAccount->addTransaction(t3);
	}
	
	[contentView setDelegate:self];
	[contentView setAutoresizesOutlineColumn:NO];
	
	[self buildIndexTree];
	[self buildContentTree];	
}

- (void)buildIndexTree
{
	[m_aIndexItems removeAllObjects];
	
	std::vector<Account>::iterator it = m_aAccounts.begin();
	
	for (; it != m_aAccounts.end(); ++it)
	{
		IndexItem *newAccount = [[IndexItem alloc] init];
		
		std::string strName = it->getName();
		NSString *sName = [[NSString alloc] initWithUTF8String:strName.c_str()];
		
		std::string strBalance = it->getBalance(true);
		NSString *sBalance = [[NSString alloc] initWithUTF8String:strBalance.c_str()];
		
		[newAccount setValue:sName forKey:@"Name"];
		[newAccount setValue:sBalance forKey:@"Balance"];
	
		[m_aIndexItems addObject:newAccount];
	}
	
	[indexView reloadData];	
}


- (void)buildContentTree
{
	[m_aContentItems removeAllObjects];
	
	fixed localBalance = 0.0;
	
	int nTransactionsToShow = 100;
	
	std::vector<Transaction>::iterator it = m_pAccount->begin();
	int nTransaction = 0;
	
	if (m_pAccount->getTransactionCount() > nTransactionsToShow)
	{
		// calculate balance without outputting it
		std::vector<Transaction>::iterator stopIt = m_pAccount->end() - nTransactionsToShow;
		for (; it != stopIt; ++it, nTransaction++)
		{
			localBalance += (*it).Amount();
		}
	}	
	
	for (; it != m_pAccount->end(); ++it, nTransaction++)
	{
		IndexItem *newTransaction = [[IndexItem alloc] init];
		
		std::string strPayee = it->Payee();
		NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
		
		std::string strDescription = it->Description();
		NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
		
		std::string strCategory = it->Category();
		NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
		
		std::string strAmount = it->Amount();
		NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
		
		std::string strDate = it->Date1().FormattedDate(0);
		NSString *sDate = [[NSString alloc] initWithUTF8String:strDate.c_str()];
		
		localBalance += it->Amount();
		
		std::string strBalance = localBalance;
		NSString *sBalance = [[NSString alloc] initWithUTF8String:strBalance.c_str()];
		
		[newTransaction setTransaction:nTransaction];
		
		int recon = it->isReconciled();
		
		[newTransaction setIntValue:recon forKey:@"Reconciled"];
		[newTransaction setValue:sDate forKey:@"Date"];
		[newTransaction setValue:sPayee forKey:@"Payee"];
		[newTransaction setValue:sDescription forKey:@"Description"];
		[newTransaction setValue:sCategory forKey:@"Category"];
		[newTransaction setValue:sAmount forKey:@"Amount"];
		[newTransaction setValue:sBalance forKey:@"Balance"];
		
		[newTransaction setIntValue:nTransaction forKey:@"Transaction"];
		
		if (it->Split())
		{
			int nSplits = it->getSplitCount();
			
			fixed splitValue = it->Amount();
			
			for (int i = 0; i < nSplits; i++)
			{
				SplitTransaction & split = it->getSplit(i);
				
				IndexItem *newSplit = [[IndexItem alloc] init];
				
				std::string strPayee = split.Payee();
				NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
				
				std::string strDescription = split.Description();
				NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
				
				std::string strCategory = split.Category();
				NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
				
				std::string strAmount = split.Amount();
				NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
				
				[newSplit setValue:sPayee forKey:@"Payee"];
				[newSplit setValue:sDescription forKey:@"Description"];
				[newSplit setValue:sCategory forKey:@"Category"];
				[newSplit setValue:sAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:i];
				[newSplit setIntValue:i forKey:@"Split"];
				
				splitValue -= split.Amount();
				
				[newTransaction addChild:newSplit];
			}
			
			// add remainder as a temp editable row
			
			if (splitValue != 0.0)
			{
				IndexItem *newSplit = [[IndexItem alloc] init];
				
				std::string strAmount = splitValue;
				NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
				
				[newSplit setValue:@"Split Value" forKey:@"Description"];
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:sAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:-2];
				[newSplit setIntValue:-2 forKey:@"Split"];
				
				[newTransaction addChild:newSplit];
			}
		}		
		
		[m_aContentItems addObject:newTransaction];		
	}
	
	[contentView reloadData];
}

- (IBAction)AddAccount:(id)sender
{
	
}

- (IBAction)AddTransaction:(id)sender
{
	Transaction newTransaction("", "", "", 0.0, -1);
	
	m_pAccount->addTransaction(newTransaction);
	
	[Payee setStringValue:@""];
	[Description setStringValue:@""];
	[Category setStringValue:@""];
	[Amount setStringValue:@""];
	
	IndexItem *newIndex = [[IndexItem alloc] init];
	
	std::string strPayee = newTransaction.Payee();
	NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
	
	std::string strDescription = newTransaction.Description();
	NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
	
	std::string strCategory = newTransaction.Category();
	NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
	
	std::string strAmount = newTransaction.Amount();
	NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
	
	std::string strDate = newTransaction.Date1().FormattedDate(0);
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
	[contentView reloadData];
	
	NSInteger row = [contentView rowForItem:newIndex];
	
	[contentView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	
	[window makeFirstResponder:Payee];
	
	m_UnsavedChanges = true;
}

- (IBAction)Delete:(id)sender
{
	NSIndexSet *rows = [contentView selectedRowIndexes];
	
	if ([rows count] > 0)
	{
		NSInteger row = [rows lastIndex];
		
		while (row != NSNotFound)
		{
			IndexItem *item = [contentView itemAtRow:row];
			
			int nSplit = [item intKeyValue:@"Split"];
			int nTransaction = [item intKeyValue:@"Transaction"];
			
			if (nSplit == -1)
			{
				m_pAccount->deleteTransaction(nTransaction);
				[m_aContentItems removeObjectAtIndex:nTransaction];
			}
			else if (nSplit != -2)
			{
				IndexItem *transactionItem = [m_aContentItems objectAtIndex:nTransaction];
				Transaction &trans = m_pAccount->getTransaction(nTransaction);
				
				trans.deleteSplit(nSplit);
				[transactionItem deleteChild:nSplit];
			}
			
			row = [rows indexLessThanIndex:row];			
		}
		
		// TODO: for multiple selection this won't work, as indexes will get out of sync
		//		 also, when removing items, the balance value of other objects will be wrong
		
//		[self buildContentTree];
		[contentView reloadData];
		
		m_UnsavedChanges = true;
	}	
}

- (IBAction)SplitTransaction:(id)sender
{
	NSInteger row = [contentView selectedRow];
	
	if (row == NSNotFound)
		return;
	
	IndexItem *item = [contentView itemAtRow:row];
	
	int nTransaction = [item intKeyValue:@"Transaction"];
	Transaction &trans = m_pAccount->getTransaction(nTransaction);
	
	fixed splitValue = trans.Amount();
	IndexItem *newSplit = [[IndexItem alloc] init];
	
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
	
	[contentView reloadData];
	
	[contentView expandItem:item];
	
	row = [contentView rowForItem:newSplit];
	
	[contentView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	
	[window makeFirstResponder:Payee];
	
	m_UnsavedChanges = true;
}

- (void)TransactionSelectionDidChange:(NSNotification *)notification
{
	NSIndexSet *rows = [contentView selectedRowIndexes];
	
	if ([rows count] == 1)
	{
		NSInteger row = [rows lastIndex];
		
		IndexItem *item = [contentView itemAtRow:row];
		
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
			
			Date date1 = trans->Date1();
			NSDate *datetemp = convertToNSDate(&date1);
			
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
		}
		else if (trans && split && nSplit != -2)
		{
			m_bEditing = true;
			
			std::string strPayee = split->Payee();
			NSString *sPayee = [[NSString alloc] initWithUTF8String:strPayee.c_str()];
			
			std::string strDescription = split->Description();
			NSString *sDescription = [[NSString alloc] initWithUTF8String:strDescription.c_str()];
			
			std::string strCategory = trans->Category();
			NSString *sCategory = [[NSString alloc] initWithUTF8String:strCategory.c_str()];
			
			std::string strAmount = split->Amount();
			NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
			
			[Payee setStringValue:sPayee];
			[Description setStringValue:sDescription];
			[Category setStringValue:sCategory];
			[Amount setStringValue:sAmount];
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
	std::string strDate = date1.FormattedDate(0);
	NSString *sDate = [[NSString alloc] initWithUTF8String:strDate.c_str()];
	
	std::string strPayee = [[Payee stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strDesc = [[Description stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	std::string strCategory = [[Category stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	double dAmount = [Amount doubleValue];
	
	fixed fAmount = dAmount;
	std::string strAmount = fAmount;
	NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
	
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
		trans->setReconciled(bReconciled);
		
		[m_SelectedTransaction setValue:sDate forKey:@"Date"];
	}
	else
	{
		if (nSplit != -2)
		{
			split->setPayee(strPayee);
			split->setDescription(strDesc);
			split->setCategory(strCategory);
			split->setAmount(fAmount);
		}
		
		if (nSplit == -2) // Dummy value, so convert to a real split
		{
			fixed transValue = trans->Amount();			
			
			trans->addSplit(strDesc, strPayee, strCategory, fAmount);
			
			int nSplitsNumber = trans->getSplitCount() - 1;
			[m_SelectedTransaction setSplitTransaction:nSplitsNumber];
			[m_SelectedTransaction setIntValue:nSplitsNumber forKey:@"Split"];
			
			fixed splitValue = trans->getSplitTotal();
			
			fixed diff = transValue -= splitValue;
			
			// Then add a new dummy value if needed
			
			if (diff != 0.0)
			{
				IndexItem *transIndex = [m_aContentItems objectAtIndex:nTrans];
				
				IndexItem *newSplit = [[IndexItem alloc] init];
				
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
			}			
		}		
	}	
	
	[m_SelectedTransaction setValue:[Payee stringValue] forKey:@"Payee"];
	[m_SelectedTransaction setValue:[Description stringValue] forKey:@"Description"];
	[m_SelectedTransaction setValue:[Category stringValue] forKey:@"Category"];
	[m_SelectedTransaction setValue:sAmount forKey:@"Amount"];
	
	[contentView reloadData];
	
	m_UnsavedChanges = true;
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
	if (item == nil)
	{
		if (outlineView == contentView)
		{
			return [m_aContentItems objectAtIndex:index];
		}
		else
		{
			return [m_aIndexItems objectAtIndex:index];
		}
	}
	else
	{
		return [(IndexItem*)item childAtIndex:index];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [item expandable];
}

-(int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (item == nil)
	{
		if (outlineView == contentView)
		{
			return [m_aContentItems count];
		}
		else
		{
			return [m_aIndexItems count];
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
		if (outlineView == contentView)
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
	if (outlineView != contentView)
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
		
		std::fstream fileStream(strFile.c_str(), std::ios::in | std::ios::binary);
		
		if (!fileStream)
		{
			NSAlert *alert = [[NSAlert alloc] init];
			[alert setMessageText:[NSString stringWithFormat:@"Could not open file: \"%@\".", fileToOpen]];
			[alert setInformativeText: @"There was a problem open the Document file."];
			[alert setAlertStyle: NSWarningAlertStyle];
			[alert addButtonWithTitle: @"OK"];
			
			[alert runModal];
			[alert release];
			return;
		}
		
		m_aAccounts.clear();
		unsigned char numAccounts = 0;
		
		fileStream.read((char *) &numAccounts, sizeof(unsigned char));
		
		for (int i = 0; i < numAccounts; i++)
		{
			Account tempAccount;
			tempAccount.Load(fileStream);
			
			m_aAccounts.push_back(tempAccount);
		}
		
		fileStream.close();
		
		m_DocumentFile = strFile;
		m_UnsavedChanges = false;
		
		[self buildContentTree];
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
		[self SaveFileTo:m_DocumentFile];
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
			m_DocumentFile = strFile;
			m_UnsavedChanges = false;
		}
	}
}

- (bool)SaveFileTo:(std::string)path
{
	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::binary);
	
	if (!fileStream)
	{
		return false;
	}
	
	unsigned char numAccounts = m_aAccounts.size();
	
	fileStream.write((char *) &numAccounts, sizeof(unsigned char));
	
	for (std::vector<Account>::iterator it = m_aAccounts.begin(); it != m_aAccounts.end(); ++it)
	{
		(*it).Store(fileStream);
	}
	
	fileStream.close();
	
	return true;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    if (m_UnsavedChanges)
    {
        NSString * message = @"You have unsaved changes in this document. Are you sure you want to quit?";
		
        NSBeginAlertSheet(@"Are you sure you want to quit?", @"Quit", @"Cancel", nil, window, self,
						@selector(quitSheetDidEnd:returnCode:contextInfo:), nil, nil, message);
        return NSTerminateLater;
    }
	
    return NSTerminateNow;
}

- (void)quitSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    [NSApp replyToApplicationShouldTerminate:returnCode == NSAlertDefaultReturn];
}

@end
