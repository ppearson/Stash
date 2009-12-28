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

#import <Cocoa/Cocoa.h>


@interface AreaChartView : NSView
{
	NSMutableArray *m_aData;
	NSMutableArray *m_aColours;
	
	NSMutableArray *m_aDates;
	NSMutableArray *m_aPlotShapes;
	
	double m_dMaxValue;
	int m_selectedPlot;
	NSString *m_sLongestDate;
}

- (void)setData:(NSMutableArray*)data;
- (void)setMaxValue:(double)value;

- (void)setDates:(NSMutableArray*)dates;
- (void)setLongestDate:(NSString*)longestDate;

@end
