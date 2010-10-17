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

#import "ValueFormatter.h"

@implementation ValueFormatter

static ValueFormatter *gSharedInterface = nil;

+ (ValueFormatter*)sharedInterface
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
		[NSNumberFormatter setDefaultFormatterBehavior:NSNumberFormatterBehavior10_4];
		
		m_numberFormatterCurrency = [[NSNumberFormatter alloc] init];
		[m_numberFormatterCurrency setNumberStyle:NSNumberFormatterCurrencyStyle];
		
		m_numberFormatterPlain = [[NSNumberFormatter alloc] init];
		[m_numberFormatterPlain setNumberStyle:NSNumberFormatterDecimalStyle];
		
		[m_numberFormatterCurrency setLenient:YES];
		[m_numberFormatterPlain setLenient:YES];
	}
	
	return self;
}

- (void)dealloc
{
	[m_numberFormatterCurrency release];
	[m_numberFormatterPlain release];
	
	[super dealloc];
}

- (NSString*)currencyStringFromNumber:(NSNumber*)number
{
	NSString* result = [m_numberFormatterCurrency stringFromNumber:number];
	
	return result;
}

- (NSString*)currencyStringFromFixed:(fixed)value
{
	return [m_numberFormatterCurrency stringFromNumber:[NSNumber numberWithDouble:value.ToDouble()]];
}

- (fixed)fixedFromString:(NSString*)value
{
	NSNumber *nResult = [m_numberFormatterCurrency numberFromString:value];
	
	if (nResult == nil)
	{
		nResult = [m_numberFormatterPlain numberFromString:value];
	}
	
	fixed amount = [nResult doubleValue];
	
	return amount;
}


@end
