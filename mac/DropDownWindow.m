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

#import "DropDownWindow.h"

@interface DropDownWindow (Private)

+ (void)dropDownWindow:(DropDownWindow*)window atPoint:(NSPoint)point withEvent:(NSEvent*)event sender:(id)sender;

@end

@implementation DropDownWindow

+ (DropDownWindow*)dropDownWindowWithContainedView:(NSView*)view
{
	DropDownWindow *ddW = [[DropDownWindow alloc] initWithContentRect1:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered
																defer:YES];
	[ddW setReleasedWhenClosed:NO];
	
	if (view)
		[ddW setContainedView:view sizeToFit:YES];
	
	return [ddW autorelease];
}

- (void)setContainedView:(NSView*)view sizeToFit:(BOOL)sizeToFit
{
	[view retain];
	[fContainedView release];
	fContainedView = view;
	
	[[self contentView] addSubview:view];
	
	if (sizeToFit)
	{
		NSRect frame = [self frameRectForContentRect:NSInsetRect([view frame], 0, 0)];
		
		frame.origin = NSZeroPoint;
		[view setFrameOrigin:NSMakePoint(0, -20)];
		[self setFrame:frame display:YES];
	}
	
	[fContainedView setNeedsDisplay:YES];
	
	if ([view isKindOfClass:[NSControl class]])
		[self setHandleFirstClick:NO];	
}

+ (void)dropDownWindow:(DropDownWindow*)window atPoint:(NSPoint)point withEvent:(NSEvent*)event sender:(id)sender
{
	if (!window)
		window = [DropDownWindow dropDownWindowWithContainedView:nil];
	
	[window retain];
	
	point = [[event window] convertPointToScreen:point];
	
	// if the popup window is going to fall off the screen, move it
	NSRect screenRect = [[NSScreen mainScreen] frame];
	NSRect buttonRect = [sender frame];
	
	if ((point.x + [window frame].size.width) >= screenRect.size.width)
	{
		point.x -= [window frame].size.width;
	}
	
	if (point.y - [window frame].size.height < 0)
	{
		point.y += [window frame].size.height + buttonRect.size.height;
	}	
	
	[window setFrameTopLeftPoint:point];
	
	[[event window] addChildWindow:window ordered:NSWindowAbove];
	
	[window orderFront:self];
	
	[window trackWithEvent:event];
	
	[[window parentWindow] removeChildWindow:window];
	[window orderOut:window];
	
	[window release];
	window = nil;
}

- (void)dropDownAtPoint:(NSPoint)point withEvent:(NSEvent*)event sender:(id)sender
{
	[[self class] dropDownWindow:self atPoint:point withEvent:event sender:sender];
}

- (void)setHandleFirstClick:(BOOL)handle
{
	bHandleFirstClick = handle;
}

- (void)setShouldCloseWhenViewTrackingReturns:(BOOL)closeOnReturn
{
	bCloseOnReturn = closeOnReturn;
}

- (NSView*)containedView
{
	return fContainedView;
}

- (id)initWithContentRect1:(NSRect)contentRect styleMask:(unsigned int)styleMask backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
	if ((self = [super initWithContentRect:contentRect styleMask:styleMask backing:bufferingType defer:deferCreation]) != nil )
	{
		[self setLevel:NSPopUpMenuWindowLevel];
		[self setHasShadow:YES];
		[self setAlphaValue:0.95];
		[self setReleasedWhenClosed:YES];
		[self setFrame:NSMakeRect(0, 0,  128, 128) display:NO];
		
		fContainedView = nil;
		bHandleFirstClick = YES;
		bCloseOnReturn = YES;
	}
	
	return self;
}

- (void)trackWithEvent:(NSEvent*)event
{
	NSTimeInterval startTime = [event timestamp];
	
	[self setAcceptsMouseMovedEvents:YES];
	
	if (bHandleFirstClick)
	{
		[[self containedView] mouseDown:event];
		
		if ([[self currentEvent] timestamp] - startTime > 0.25)
			return;
	}
	
	NSEvent *thisEvent;
	BOOL maintain = YES;
	unsigned int mask;
	
	mask = NSLeftMouseUpMask | NSLeftMouseDraggedMask |
	NSRightMouseUpMask | NSRightMouseDraggedMask | NSAppKitDefined |
	NSFlagsChangedMask | NSScrollWheelMask | NSKeyDownMask;
	
	while (maintain)
	{
		thisEvent = [self nextEventMatchingMask:mask];
		
		switch([thisEvent type])
		{
			case NSMouseMoved:
				[[self containedView] mouseMoved:thisEvent];
				break;				
			case NSRightMouseUp:
			case NSLeftMouseUp:
				if  ([thisEvent timestamp] - startTime < 0.25 || !bHandleFirstClick)
				{
					mask |= (NSLeftMouseDownMask | NSRightMouseDownMask | NSMouseMovedMask);
				}
				else
				{
					[[self containedView] mouseUp:thisEvent];
					maintain = NO;
				}
				break;
			case NSRightMouseDown:
			case NSLeftMouseDown:
				if (!NSPointInRect([thisEvent locationInWindow], [[self containedView] frame]))
					maintain = NO;
				else
				{
					[[self containedView] mouseDown:thisEvent];
					
					if (bCloseOnReturn)
						maintain = NO;
				}
				break;
			case NSPeriodic:
				break;
			case NSFlagsChanged:
				[[self containedView] flagsChanged:thisEvent];
				break;
			case NSAppKitDefined:
				if ([thisEvent subtype] == NSApplicationDeactivatedEventType)
					maintain = NO;
				break;
			case NSKeyDown:
				if ([thisEvent keyCode] == 53) // Escape key
					maintain = NO;
				break;
			default:
				break;
		}		
	}
	
	[self discardEventsMatchingMask:NSAnyEventMask beforeEvent:thisEvent];
}

@end

@implementation NSEvent (DropDownWindow)
- (BOOL)isWantedEvent
{
	NSEventType t = [self type];
	
	return ( t == NSLeftMouseDown		||
			t == NSLeftMouseUp			||
			t == NSRightMouseDown		||
			t == NSRightMouseUp		||
			t == NSLeftMouseDragged	||
			t == NSRightMouseDragged   ||
			t == NSOtherMouseDown		||
			t == NSOtherMouseUp		||
			t == NSOtherMouseDragged ||
			t == NSKeyDown);
}
@end

