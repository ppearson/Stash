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

#include "settings_window.h"

#if STASH_QT_VER < 5
#include <QtGui>
#else
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#endif
#include <QSettings>

#include "general_page.h"
#include "transactions_page.h"
#include "pie_chart_page.h"
#include "area_chart_page.h"

SettingsWindow::SettingsWindow(QSettings& settings, QWidget* parent) : QDialog(parent), m_settings(settings)
{
	m_categoryWidget = new QListWidget();
	m_categoryWidget->setMovement(QListView::Static);
	m_categoryWidget->setMaximumWidth(160);

	m_pagesWidget = new QStackedWidget();

	QPushButton* OKButton = new QPushButton(tr("OK"));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	QHBoxLayout* horizontalLayout = new QHBoxLayout();
	horizontalLayout->addWidget(m_categoryWidget, 0, Qt::AlignLeft);
	horizontalLayout->addWidget(m_pagesWidget, 1);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(OKButton);
	buttonsLayout->addWidget(cancelButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(horizontalLayout, 0);
	mainLayout->addLayout(buttonsLayout, 0);
	setLayout(mainLayout);

	resize(800, 500);

	connect(OKButton, SIGNAL(clicked()), this, SLOT(saveChanges()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

	connect(m_categoryWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
				 this, SLOT(changePage(QListWidgetItem*, QListWidgetItem*)));

	setWindowTitle(tr("Stash Settings"));

	addPage("General", new GeneralPage(m_settings));
	addPage("Transactions", new TransactionsPage(m_settings));
	addPage("Pie Chart", new PieChartPage(m_settings));
	addPage("Area Chart", new AreaChartPage(m_settings));

	m_categoryWidget->setCurrentRow(0);
}

void SettingsWindow::saveChanges()
{
	std::vector<SettingsPage*>::iterator it = m_aPages.begin();
	for (; it != m_aPages.end(); ++it)
	{
		(*it)->saveSettings();
	}

	accept();
}

void SettingsWindow::addPage(const QString& title, SettingsPage* page)
{
	QListWidgetItem* newPageItem = new QListWidgetItem(m_categoryWidget);
	newPageItem->setText(title);

	m_pagesWidget->addWidget(page);

	m_aPages.emplace_back(page);
}

void SettingsWindow::changePage(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (!current)
		current = previous;

	int row = m_categoryWidget->row(current);

	m_pagesWidget->setCurrentIndex(row);
}
