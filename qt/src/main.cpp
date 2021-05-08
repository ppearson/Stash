/*
 * Stash:  A Personal Finance app (Qt UI).
 * Copyright (C) 2020-2021 Peter Pearson
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

#ifndef STASH_QT_5
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif

#include "stash_window.h"

int main(int argc, char** argv)
{
	// launch the GUI
	QApplication a(argc, argv);

#if __APPLE__
#ifdef STASH_QT_5
	a.setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	a.setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#endif
#endif
	QCoreApplication::setOrganizationName("peterpearson");
	QCoreApplication::setApplicationName("Stash");

	StashWindow w;
	w.show();
	
	if (argc > 1)
	{
		// second arg is the filename to open
		std::string fileName(argv[1]);
		w.openDocument(fileName);
	}

	return a.exec();
}
