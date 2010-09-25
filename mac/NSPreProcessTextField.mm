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

#include <string>
#include "expression_parser.h"
#include "fixed.h"

#import "NSPreProcessTextField.h"

@implementation NSPreProcessTextField

- (void)awakeFromNib
{	
	[self setDelegate:self];
}

// captures Enter on Amount field for inline calculations, and parses the expression,
// replacing the text with the number result
- (BOOL)control:(NSControl*)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command
{
	BOOL result = NO;
	
	if (command == @selector(insertNewline:)) // if enter was hit, do our stuff
	{
		std::string strText = [[self stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		
		if (!strText.empty() && strText.substr(0, 1) == "=")
		{
			double dResult = 0.0;
			if (parse(strText, dResult))
			{
				fixed fResult = dResult;
				
				std::string strResult = fResult;
				NSString *sResult = [[NSString alloc] initWithUTF8String:strResult.c_str()];
				
				[self setStringValue:sResult];
				
				result = YES;
			}
		}			
	}
	
	return result;
}

@end
