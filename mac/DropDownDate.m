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

#import "DropDownDate.h"
#import "DropDownWindow.h"

@implementation DropDownDate

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
        // Initialization code here.
    }
    return self;
}

- (void)mouseDown:(NSEvent*)event
{
	fWindow = [DropDownWindow dropDownWindowWithContainedView:fContainedView];
	if (fWindow)
	{
		[fWindow setShouldCloseWhenViewTrackingReturns:NO];
		
		[fContainedView setTarget:self];
		[fContainedView setAction:@selector(dropDownDateChanged:)];
		
		[fContainedView setDateValue:[fLinkedItem dateValue]];
		
		NSPoint p = [self bounds].origin;
		p = [self convertPoint:p toView:nil];
		
		NSRect rect = [self bounds];
		p.y -= rect.size.height;
		
		[fWindow dropDownAtPoint:p withEvent:event sender:self];
	}
}

- (IBAction)dropDownDateChanged:(id)sender
{
	NSDate *date = [sender dateValue];
	
	[fLinkedItem setDateValue:date];
	
	[fWindow setShouldCloseWhenViewTrackingReturns:YES];
}

@end
