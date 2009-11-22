//
//  PreferencesWindow.m
//  Stash
//
//  Created by Peter Pearson on 22/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "PreferencesWindow.h"


@implementation PreferencesWindow

- (void) keyDown: (NSEvent *) event
{
    if ([event keyCode] == 53) // esc key
        [self close];
    else
        [super keyDown: event];
}

- (void) close
{
    [self makeFirstResponder: nil];
    [super close];
}

@end
