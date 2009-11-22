//
//  PreferencesController.m
//  Stash
//
//  Created by Peter Pearson on 22/11/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "PreferencesController.h"


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

}

@end
