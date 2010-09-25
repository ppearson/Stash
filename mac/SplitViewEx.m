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

#import "SplitViewEx.h"

@implementation NSSplitView (SplitViewEx)

- (void)loadLayoutFromDefault:(NSString*)name
{
	NSString *sRectKey1 = [NSString stringWithFormat:@"%@-view1", name];
	NSString *sRectKey2 = [NSString stringWithFormat:@"%@-view2", name];
	
	NSString *sRect1 = [[NSUserDefaults standardUserDefaults] objectForKey:sRectKey1];
	NSString *sRect2 = [[NSUserDefaults standardUserDefaults] objectForKey:sRectKey2];
	
	if (sRect1 != nil && sRect2 != nil)
	{
		NSRect subviewRect0 = NSRectFromString(sRect1);
		NSRect subviewRect1 = NSRectFromString(sRect2);
		
		[[[self subviews] objectAtIndex:0] setFrame:subviewRect0];
		[[[self subviews] objectAtIndex:1] setFrame:subviewRect1];
	}	
}

- (void)saveLayoutToDefault:(NSString*)name
{
	NSRect subviewRect0 = [[[self subviews] objectAtIndex:0] frame];
	NSRect subviewRect1 = [[[self subviews] objectAtIndex:1] frame];
	
	NSString *sRectKey1 = [NSString stringWithFormat:@"%@-view1", name];
	NSString *sRectKey2 = [NSString stringWithFormat:@"%@-view2", name];
	
	NSString *sRect1 = NSStringFromRect(subviewRect0);
	NSString *sRect2 = NSStringFromRect(subviewRect1);
	
	[[NSUserDefaults standardUserDefaults] setObject:sRect1 forKey:sRectKey1];
	[[NSUserDefaults standardUserDefaults] setObject:sRect2 forKey:sRectKey2];	
}

@end
