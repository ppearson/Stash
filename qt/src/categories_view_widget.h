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

#ifndef CATEGORIES_VIEW_WIDGET_H
#define CATEGORIES_VIEW_WIDGET_H

#include <QWidget>

class QListWidget;
class QListWidgetItem;

class Document;

class ItemControlButtonsWidget;

class StashWindow;

// Note: CategoriesViewWidget and PayeesViewWidget are very similar and there's a lot of
//       duplication, however given there are only two types, it didn't seem worth abstracting...

class CategoriesViewWidget : public QWidget
{
	Q_OBJECT
public:
	CategoriesViewWidget(QWidget* pParent, StashWindow* mainWindow);
	virtual ~CategoriesViewWidget();
	
	void updateCategoriesFromDocument();
	
public slots:
	void addItemClicked();
	void deleteItemClicked();
	
	void itemChanged(QListWidgetItem* item);
	void currentRowChanged(int currentRow);
	
protected:
	StashWindow*				m_pMainWindow;
	
	QListWidget*				m_pListWidget;
	
	ItemControlButtonsWidget*	m_pItemControlButtons;
};

#endif // CATEGORIES_VIEW_WIDGET_H
