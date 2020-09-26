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

#ifndef ITEM_CONTROL_BUTTONS_WIDGET_H
#define ITEM_CONTROL_BUTTONS_WIDGET_H

#include <QWidget>

class QPushButton;
class QIcon;

class ItemControlButtonsWidget : public QWidget
{
	Q_OBJECT
public:
	
	enum Type
	{
		eTransaction,
		ePayee,
		eCategory,
		eScheduledTransaction,
		eGraph
	};
	
	enum EnabledButtonFlags
	{
		eBtnAdd			= 1 << 0,
		eBtnDelete		= 1 << 1,
		eBtnSplit		= 1 << 2,
		eBtnUp			= 1 << 3,
		eBtnDown		= 1 << 4
	};
	
	ItemControlButtonsWidget(Type type, QWidget* pParent);
	virtual ~ItemControlButtonsWidget();
	
	void setButtonsEnabled(unsigned int buttonsEnabled);
	
signals:
	void addItemButtonClicked();
	void deleteItemButtonClicked();
	void splitItemButtonClicked();
	void moveUpItemButtonClicked();
	void moveDownItemButtonClicked();
	
public slots:
	void addClicked();
	void deleteClicked();
	void splitClicked();
	void moveUpClicked();
	void moveDownClicked();
	
protected:
	
	QPushButton* createNewButton(const QIcon& icon, const QString& tooltip);

protected:
	
	Type				m_type;
	
	QPushButton*		m_pAddItemButton;
	QPushButton*		m_pDeleteItemButton;
	QPushButton*		m_pSplitItemButton;
	QPushButton*		m_pMoveUpItemButton;
	QPushButton*		m_pMoveDownItemButton;
};

#endif // ITEM_CONTROL_BUTTONS_WIDGET_H
