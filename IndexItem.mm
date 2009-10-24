
#import "IndexItem.h"

@implementation IndexItem

- (id)init
{
	[super init];
	children = [[NSMutableArray alloc] init];
	data = [[NSMutableDictionary alloc] init];
	
	m_trans = -1;
	m_split = -1;
	
	return self;
}

- (int)transaction
{
	return m_trans;
}

- (void)setTransaction:(int)transaction
{
	m_trans = transaction;
}

- (int)splitTransaction
{
	return m_split;
}

- (void)setSplitTransaction:(int)stransaction
{
	m_split = stransaction;
}

- (void)setValue:(NSString *)value forKey:(NSString *)key
{
	[data setValue:value forKey:key];
}

- (NSString *)keyValue:(NSString *)key
{
	if ([data objectForKey:key] != nil)
		return [data valueForKey:key];
	
	return @"";
}

- (int)intKeyValue:(NSString *)key
{
	if ([data objectForKey:key] != nil)
		return [[data valueForKey:key] intValue];
	
	return -1;
}

- (void)setIntValue:(int)value forKey:(NSString *)key
{
	[data setValue:[NSNumber numberWithInt:value] forKey:key];
}

- (void)addChild:(IndexItem *)n
{
	[children addObject:n];
}

- (IndexItem *)childAtIndex:(int)i
{
	return [children objectAtIndex:i];
}

- (int)childrenCount
{
	return [children count];
}

- (BOOL)expandable
{
	return ([children count] > 0);
}

- (void)dealloc
{
	[children release];
	[data release];
	[super dealloc];
}

@end
