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

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <QDialog>

#include <vector>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QSettings;

class SettingsPage;

class SettingsWindow : public QDialog
{
	Q_OBJECT
public:
	explicit SettingsWindow(QSettings& settings, QWidget* parent = 0);

	void addPage(const QString& title, SettingsPage* page);

signals:

public slots:
	void changePage(QListWidgetItem* current, QListWidgetItem* previous);
	void saveChanges();

protected:
	QSettings&			m_settings;

	QListWidget*		m_categoryWidget;
	QStackedWidget*		m_pagesWidget;

	std::vector<SettingsPage*>	m_aPages;
};

#endif // SETTINGS_WINDOW_H
