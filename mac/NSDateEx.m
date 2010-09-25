/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009 Peter Pearson
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

#import "NSDateEx.h"

@implementation NSDate (NSDateEx)

- (NSDate*)addWeeks:(int)weeks
{
	NSDateComponents* components = [[NSDateComponents alloc] init];
	[components setWeek:weeks];
	return [self addComponents:components];
}

- (NSDate*)addMonths:(int)months
{
	NSDateComponents* components = [[NSDateComponents alloc] init];
	[components setMonth:months];
	return [self addComponents:components];
}

- (NSDate*)addYears:(int)years
{
	NSDateComponents* components = [[NSDateComponents alloc] init];
	[components setYear:years];
	return [self addComponents:components];
}

- (NSDate*)firstDayOfMonth
{
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
	NSDateComponents *components = [calendar components:(NSYearCalendarUnit | NSMonthCalendarUnit) fromDate:self];
	[components setDay:1];
	return [calendar dateFromComponents:components];
}

- (NSDate*)lastDayOfMonth
{
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
	NSDateComponents *components = [calendar components:(NSYearCalendarUnit | NSMonthCalendarUnit) fromDate:self];
	[components setDay:0];
	[components setMonth:components.month + 1];
	return [calendar dateFromComponents:components];
}

- (NSDate*)firstDayOfYear
{
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
	NSDateComponents *components = [calendar components:(NSYearCalendarUnit | NSMonthCalendarUnit) fromDate:self];
	[components setDay:1];
	[components setMonth:1];
	return [calendar dateFromComponents:components];
}

- (NSDate*)lastDayOfYear
{
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
	NSDateComponents *components = [calendar components:(NSYearCalendarUnit | NSMonthCalendarUnit) fromDate:self];
	[components setDay:31];
	[components setMonth:12];
	[components setYear:components.year];
	return [calendar dateFromComponents:components];
}

- (NSDate*)addComponents:(NSDateComponents*)components
{
	return [[NSCalendar currentCalendar] dateByAddingComponents:components toDate:self options:0];
}

@end
