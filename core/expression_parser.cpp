/* 
 * Stash:  A Personal Finance app (core).
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

// very simple maths expression parser
// only copes with + and -

#include <stdlib.h>
#include <string>
#include <queue>
#include <algorithm>

#include "expression_parser.h"

bool parse(const std::string &expr, double &result)
{
	std::queue<double> aValues;
	std::queue<char> aOperators;

	std::string expression = expr;

	// remove spaces
	expression.erase(std::remove_if(expression.begin(), expression.end(), ::isspace), expression.end());

	if (expression.empty())
		return false;

	if (expression.substr(0, 1) == "=")
	{
		expression = expression.substr(1);
	}

	size_t currPos = 0;
	size_t nextOpPos = 0;

	while (true)
	{
		nextOpPos = expression.find_first_of("+-", currPos);

		if (nextOpPos == std::string::npos)
		{
			if (currPos == 0)
				break;
			
			// remainder should be a number
			std::string strValue = expression.substr(currPos);
			double dValue = atof(strValue.c_str());
			aValues.push(dValue);

			break;
		}
		else if (nextOpPos == 0) // first char is an operator, must be a sign for the number
		{
			nextOpPos = expression.find_first_of("+-", 1);

			if (nextOpPos == std::string::npos)
				break;
		}
		else if (nextOpPos == currPos) // two operators together before the end - last one will be sign
		{
			nextOpPos = expression.find_first_of("+-", currPos + 1);
		}
		
		// get the number value before it
		std::string strValue = expression.substr(currPos, nextOpPos - currPos);
		double dValue = atof(strValue.c_str());
		aValues.push(dValue);

		aOperators.push(expression[nextOpPos]);

		currPos = nextOpPos + 1;
	}

	// now use queues to work out result

	double dResult = 0.0;

	if (aOperators.empty() || aValues.size() < 2)
	{
		// haven't got enough data
		return false;
	}

	dResult = aValues.front();
	aValues.pop();

	while (!aOperators.empty())
	{
		char cOperator = aOperators.front();
		aOperators.pop();

		double dVal = aValues.front();
		aValues.pop();

		if (cOperator == '+')
		{
			dResult += dVal;
		}
		else if (cOperator == '-')
		{
			dResult -= dVal;
		}
	}

	result = dResult;

	return true;
}
