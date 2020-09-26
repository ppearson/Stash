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

#ifndef GRAPH_FORM_PANEL_H
#define GRAPH_FORM_PANEL_H

#include <string>

#include <QWidget>

#include "../graph_common.h"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QDateEdit;
class QPushButton;
class QListWidget;

class Document;
class Graph;

class ItemControlButtonsWidget;
class GraphsViewWidget;

class GraphFormPanel : public QWidget
{
	Q_OBJECT
public:
	GraphFormPanel(Document& document, GraphsViewWidget* pGraphsViewWidget, QWidget* parent = 0);
	
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
	void setParamsFromGraph(const Graph& graph);
	
	void updateGraphFromParamValues(Graph& graph);
	
	TempGraphParamState getTempGraphParamValues() const;
	
protected:
	void updateAccountsList(bool selectFirst);
		
protected slots:
	void paramChanged();
	
signals:
	void graphParamValuesUpdated();
	
protected:
	Document&			m_document;
	
	// parent
	GraphsViewWidget*	m_pGraphsViewWidget;
	
	QComboBox*			m_pAccount;
	QComboBox*			m_pDataType;
	
	QCheckBox*			m_pIgnoreTransfers;
	
	QDateEdit*			m_pStartDate;
	QDateEdit*			m_pEndDate;
	
	QComboBox*			m_pItemType;
	QListWidget*		m_pItemsList;
	
	ItemControlButtonsWidget*	m_pItemControlButtons;
	
	
	QPushButton*		m_pUpdate;
};

#endif // GRAPH_FORM_PANEL_H
