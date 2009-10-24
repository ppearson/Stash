#import <Cocoa/Cocoa.h>
#include "transaction.h"
#include "split_transaction.h"


@interface IndexItem : NSObject {
	
	NSMutableArray *children;
	NSMutableDictionary *data;
	
	int m_trans;
	int m_split;
}

- (int)transaction;
- (void)setTransaction:(int)transaction;

- (int)splitTransaction;
- (void)setSplitTransaction:(int)stransaction;

- (NSString *)keyValue:(NSString *)key;
- (void)setValue:(NSString *)value forKey:(NSString *)key;

- (int)intKeyValue:(NSString *)key;
- (void)setIntValue:(int)value forKey:(NSString *)key;

- (void)addChild:(IndexItem *)n;
- (int)childrenCount;
- (IndexItem *)childAtIndex:(int)i;

- (BOOL)expandable;

@end
