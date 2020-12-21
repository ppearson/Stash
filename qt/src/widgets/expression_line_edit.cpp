/*
 * Stash:  A Personal Finance app (Qt UI).
 * Copyright (C) 2020 Peter Pearson
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

#include "expression_line_edit.h"

#include <QKeyEvent>

#include "../../core/expression_parser.h"

ExpressionLineEdit::ExpressionLineEdit(QWidget* parent) : QLineEdit(parent)
{
	
}

void ExpressionLineEdit::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		if (text().size() > 1 && text().at(0) == '=')
		{
			double valueResult = 0.0;
			std::string currentString = text().toStdString();
			
			if (parse(currentString, valueResult))
			{
				char szTemp[16];
				// TODO: do this properly...
				sprintf(szTemp, "%0.2f", valueResult);
				
				setText(QString(szTemp));
				return;
			}
		}
	}
	
	QLineEdit::keyPressEvent(event);
}
