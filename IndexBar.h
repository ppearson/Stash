//
//  IndexBar.h
//  Stash
//
//  Created by Peter Pearson on 01/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface IndexBar : NSOutlineView
{
	NSMutableArray *m_aSections;
	NSMutableDictionary *m_dItems;
}

- (void)clearAllItems;

- (void)addSection:(id)key title:(NSString*)sTitle;

- (void)addItem:(id)parentKey key:(id)key title:(NSString*)sTitle item:(int)item action:(SEL)selector target:(id)target;


- (void)expandSection:(id)itemKey;

@end
