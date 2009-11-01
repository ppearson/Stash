//
//  IndexItem.h
//  Stash
//
//  Created by Peter Pearson on 01/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface IndexItem : NSObject
{
	NSMutableArray *m_aChildren;
	id parentKey;
	id itemKey;
	
	NSString *title;
	int itemIndex;
	
	id actionTarget;
	SEL action;
}

@property (retain) NSString *title;
//@property (assign) int itemIndex;
@property (assign) id parentKey;
@property (assign) id itemKey;

@property (assign, readonly) id actionTarget;
@property (assign, readonly) SEL action;

- (void)addChild:(IndexItem *)item;

- (IndexItem *)childAtIndex:(int)index;
- (int)numberOfChildren;

- (void)setItemIndex:(int)iIndex;
- (int)getItemIndex;

- (void)setAction:(SEL)selector target:(id)target;
- (BOOL)hasAction;

@end
