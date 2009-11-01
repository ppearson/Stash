#import <Cocoa/Cocoa.h>
#include "transaction.h"
#include "split_transaction.h"


@interface TransactionItem : NSObject {
	
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

- (void)addChild:(TransactionItem *)n;
- (int)childrenCount;
- (TransactionItem *)childAtIndex:(int)i;
- (void)deleteChild:(int)i;

- (BOOL)expandable;

@end
