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

#import "PreferencesController.h"

#define TOOLBAR_GENERAL			@"TOOLBAR_GENERAL"
#define TOOLBAR_TRANSACTIONS	@"TOOLBAR_TRANSACTIONS"
#define TOOLBAR_PIECHART		@"TOOLBAR_PIECHART"
#define TOOLBAR_AREACHART		@"TOOLBAR_AREACHART"

@interface PreferencesController (Private)

- (void) setPrefView: (id) sender;

@end


@implementation PreferencesController

- (id)init
{
	if (self = [super initWithWindowNibName:@"Preferences"])
	{
		uDefaults = [NSUserDefaults standardUserDefaults];
	}
	return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)awakeFromNib
{
	bHasLoaded = YES;
    
    NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"Preferences Toolbar"];
    [toolbar setDelegate:self];
    [toolbar setAllowsUserCustomization:NO];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    [toolbar setSizeMode:NSToolbarSizeModeRegular];
    [toolbar setSelectedItemIdentifier:TOOLBAR_GENERAL];
    [[self window] setToolbar:toolbar];
    [toolbar release];
    
    [self setPrefView:nil];
	
	[fPieSegmentSort removeAllItems];
	[fPieSegmentSort addItemWithTitle:NSLocalizedString(@"Size", "Preferences -> Pie Chart -> Sort -> Size")];
	[fPieSegmentSort addItemWithTitle:NSLocalizedString(@"Title", "Preferences -> Pie Chart -> Sort -> Title")];
	
	[fPieSegmentSort selectItemAtIndex:[uDefaults integerForKey:@"PieChartSortType"]];
	
	[fPieSegmentSelectionType removeAllItems];
	[fPieSegmentSelectionType addItemWithTitle:NSLocalizedString(@"are selected", "Preferences -> Pie Chart -> Selection Type -> Selected")];
	[fPieSegmentSelectionType addItemWithTitle:NSLocalizedString(@"pop out", "Preferences -> Pie Chart -> Selection Type -> Pop Out")];
	
	[fPieSegmentSelectionType selectItemAtIndex:[uDefaults integerForKey:@"PieChartSelectionType"]];
	
	if ([uDefaults boolForKey:@"PieChartGroupSmallerItems"] == NO)
	{
		[fPieGroupSize setEnabled:NO];
		[fPieGroupName setEnabled:NO];
	}
	
	if ([uDefaults boolForKey:@"AreaChartGroupSmallerItems"] == NO)
	{
		[fAreaGroupSize setEnabled:NO];
		[fAreaGroupName setEnabled:NO];
	}
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)ident willBeInsertedIntoToolbar:(BOOL)flag
{
    NSToolbarItem * item = [[NSToolbarItem alloc] initWithItemIdentifier: ident];
	
    if ([ident isEqualToString:TOOLBAR_GENERAL])
    {
        [item setLabel:NSLocalizedString(@"General", "Preferences -> General")];
        [item setImage:[NSImage imageNamed:NSImageNamePreferencesGeneral]];
        [item setTarget:self];
        [item setAction:@selector(setPrefView:)];
        [item setAutovalidates:NO];
    }
    else if ([ident isEqualToString:TOOLBAR_TRANSACTIONS])
    {
        [item setLabel:NSLocalizedString(@"Transactions", "Preferences -> Transactions")];
        [item setImage:[NSImage imageNamed:@"trans_prefs.png"]];
        [item setTarget:self];
        [item setAction:@selector(setPrefView:)];
        [item setAutovalidates:NO];
    }
    else if ([ident isEqualToString:TOOLBAR_PIECHART])
    {
        [item setLabel:NSLocalizedString(@"Pie Chart", "Preferences -> Pie Chart")];
        [item setImage:[NSImage imageNamed:@"pie_prefs.png"]];
        [item setTarget:self];
        [item setAction:@selector(setPrefView:)];
        [item setAutovalidates:NO];
    }
    else if ([ident isEqualToString:TOOLBAR_AREACHART])
    {
        [item setLabel:NSLocalizedString(@"Area Chart", "Preferences -> Area Chart")];
        [item setImage:[NSImage imageNamed:@"area_prefs.png"]];
        [item setTarget:self];
        [item setAction:@selector(setPrefView:)];
        [item setAutovalidates:NO];
    }
	else
    {
        [item release];
        return nil;
    }
	
    return [item autorelease];
}

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar
{
    return [self toolbarDefaultItemIdentifiers:toolbar];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    return [self toolbarAllowedItemIdentifiers:toolbar];
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    return [NSArray arrayWithObjects:TOOLBAR_GENERAL, TOOLBAR_TRANSACTIONS, TOOLBAR_PIECHART, TOOLBAR_AREACHART, nil];
}

- (void)updateTransactionsSettings:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:@"TransactionsSettingsUpdate" object:self];
}

- (void)updateGraphSettings:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:@"GraphSettingsUpdate" object:self];
}

- (void)pieChartGroupItemsToggle:(id)sender
{
	if ([sender state] == NSOnState)
	{
		[fPieGroupSize setEnabled:YES];
		[fPieGroupName setEnabled:YES];
	}
	else
	{
		[fPieGroupSize setEnabled:NO];
		[fPieGroupName setEnabled:NO];
	}
	
	[self performSelector:@selector(updateGraphSettings:) withObject:nil];
}

- (void)areaChartGroupItemsToggle:(id)sender
{
	if ([sender state] == NSOnState)
	{
		[fAreaGroupSize setEnabled:YES];
		[fAreaGroupName setEnabled:YES];
	}
	else
	{
		[fAreaGroupSize setEnabled:NO];
		[fAreaGroupName setEnabled:NO];
	}
	
	[self performSelector:@selector(updateGraphSettings:) withObject:nil];
}

@end

@implementation PreferencesController (Private)

- (void)setPrefView:(id)sender
{
    NSString * identifier;
    if (sender)
    {
        identifier = [sender itemIdentifier];
        [[NSUserDefaults standardUserDefaults] setObject:identifier forKey:@"SelectedPrefView"];
    }
    else
        identifier = [[NSUserDefaults standardUserDefaults] stringForKey:@"SelectedPrefView"];
    
    NSView *view;
    if ([identifier isEqualToString:TOOLBAR_TRANSACTIONS])
        view = vTransactionsView;
    else if ([identifier isEqualToString:TOOLBAR_PIECHART])
        view = vPieChartView;
    else if ([identifier isEqualToString:TOOLBAR_AREACHART])
        view = vAreaChartView;
	else
    {
        identifier = TOOLBAR_GENERAL;
        view = vGeneralView;
    }
    
    [[[self window] toolbar] setSelectedItemIdentifier:identifier];
    
    NSWindow *window = [self window];
    if ([window contentView] == view)
        return;
    
    [window setContentView:view];
    
    if (sender)
	{
		NSString *sTitle = [NSString stringWithFormat:NSLocalizedString(@"%@ Preferences", "Preferences Window Title"), [sender label]];
        [window setTitle:sTitle];
	}
    else
    {
        NSToolbar *toolbar = [window toolbar];
        NSString *itemIdentifier = [toolbar selectedItemIdentifier];
		int items = [[toolbar items] count];
		int i = 0;
        for (i = 0; i < items; i++)
		{
			NSToolbarItem *item = [[toolbar items] objectAtIndex:i];
            if ([[item itemIdentifier] isEqualToString: itemIdentifier])
            {
				NSString *sTitle = [NSString stringWithFormat:NSLocalizedString(@"%@ Preferences", "Preferences Window Title"), [item label]];
                [window setTitle:sTitle];
                break;
            }
		}
    }
}

@end
