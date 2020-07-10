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

#ifndef SETTINGS_STATE_H
#define SETTINGS_STATE_H

#include <string>

#include <QSettings>

class SettingsState
{
public:
	SettingsState()
	{
		
	}
	
	void saveBool(const std::string& key, bool value)
	{
		m_settings.setValue(key.c_str(), value);
	}

	void saveInt(const std::string& key, int value)
	{
		m_settings.setValue(key.c_str(), value);
	}

	bool getBool(const std::string& key, bool defaultValue) const
	{
		return m_settings.value(key.c_str(), defaultValue).toBool();
	}

	int getInt(const std::string& key, int defaultValue) const
	{
		return m_settings.value(key.c_str(), defaultValue).toInt();
	}

	QSettings& getInternal() { return m_settings; }	
	
protected:
	QSettings		m_settings;
};

#endif // SETTINGS_STATE_H

