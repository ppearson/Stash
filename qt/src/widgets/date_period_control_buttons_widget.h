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

#ifndef DATE_PERIOD_CONTROL_BUTTONS_WIDGET_H
#define DATE_PERIOD_CONTROL_BUTTONS_WIDGET_H

#include <QWidget>

#include "../../core/graph.h"

class QPushButton;
class QComboBox;

class DatePeriodControlButtonsWidget : public QWidget
{
	Q_OBJECT
public:
	DatePeriodControlButtonsWidget(QWidget* pParent);

	void setType(Graph::DateType type);
	
	Graph::DateType getType() const;
		
signals:
	void previousButtonClicked();
	void nextButtonClicked();
	
	void typeIndexChanged();
	
public slots:
	void previousClicked();
	void nextClicked();
	
	void typeChanged();
	
protected:
	
	QPushButton* createNewButton(int size, const QIcon& icon, const QString& tooltip);

protected:	
	QPushButton*		m_pPreviousButton;
	QPushButton*		m_pNextButton;
	
	QComboBox*			m_pType;
};

#endif // DATE_PERIOD_CONTROL_BUTTONS_WIDGET_H
