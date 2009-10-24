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

- (void) awakeFromNib
{
	m_bEditing = false;	
	m_SelectedTransaction = 0;
	
	m_aContentItems = [[NSMutableArray alloc] init];
	
	NSDate *date1 = [NSDate date];
	[DateCntl setDateValue:date1];
	
	Account acc;
	
	m_aAccounts.push_back(acc);
	m_pAccount = &m_aAccounts[0];
	
	Transaction t0("Starting balance", "", 2142.51, Date(13, 9, 2009));
	Transaction t1("Tax", "Council", -86.00, Date(13, 9, 2009));
	Transaction t2("Food", "Sainsbury's", -13.44, Date(17, 9, 2009));
	Transaction t3("Pay", "Work", 2470.0, Date(29, 9, 2009));
	
	t1.addSplit("Test1", "Test1", -30);
	t1.addSplit("Test2", "Test2", -21.44);
	
	m_pAccount->addTransaction(t0);
	m_pAccount->addTransaction(t1);
	m_pAccount->addTransaction(t2);
	m_pAccount->addTransaction(t3);	
	
//	[contentView setDelegate:self];
	[contentView setAutoresizesOutlineColumn:NO];
	
	[self buildIndexTree];
	[self buildContentTree];	
}






- (void)buildIndexTree
{
	[m_aIndexItems removeAllObjects];
	
	
}


- (void)buildContentTree
{
	[m_aContentItems removeAllObjects];
	
	fixed localBalance = 0.0;
	
	int nTransactionsToShow = 10;
	
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
				
				std::string strAmount = split.Amount();
				NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
				
				[newSplit setValue:sPayee forKey:@"Payee"];
				[newSplit setValue:sDescription forKey:@"Description"];
				[newSplit setValue:sAmount forKey:@"Amount"];
				
				[newSplit setTransaction:nTransaction];
				[newSplit setIntValue:nTransaction forKey:@"Transaction"];
				
				[newSplit setSplitTransaction:i];
				[newSplit setIntValue:i forKey:@"Split"];
				
				splitValue -= split.Amount();
				
				[newTransaction addChild:newSplit];
			}
			
			// add remainder as a temp editable row
			
			if (splitValue < 0.0)
			{
				IndexItem *newSplit = [[IndexItem alloc] init];
				
				std::string strAmount = splitValue;
				NSString *sAmount = [[NSString alloc] initWithUTF8String:strAmount.c_str()];
				
				[newSplit setValue:@"Split Value" forKey:@"Payee"];
				[newSplit setValue:@"" forKey:@"Description"];
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
					trans->setReconciled(true);
				else
					trans->setReconciled(false);
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
		
		[cell setEnabled:enableCheck];
	}
	else
		[cell setTextColor:fontColor];
}

@end
