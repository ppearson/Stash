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

#include "about_dialog.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent)
{
	resize(500, 240);
	
	setWindowTitle("About Stash");
	
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	
	// in theory could pull this through from a shared variable in core/
	// that both Qt and MacOS versions use, but not sure it's worth it?
	QLabel* label1 = new QLabel("Stash version 1.0.1");
	// Make the font a bit bigger and bold for this label.
	QFont currFont = label1->font();
	currFont.setPointSize(currFont.pointSize() + 2);
	currFont.setBold(true);
	label1->setFont(currFont);
	mainLayout->addWidget(label1, 0, Qt::AlignHCenter);
	
	QLabel* label2 = new QLabel("(Qt UI)");
	mainLayout->addWidget(label2, 0, Qt::AlignHCenter);
	
	QLabel* label3 = new QLabel("Copyright (C) 2009-2021 Peter Pearson.");
	mainLayout->addWidget(label3, 0, Qt::AlignHCenter);
	
	QPixmap* icon = new QPixmap(":/stash/images/main_icon_2.png");
	QLabel* iconLabel = new QLabel();
	iconLabel->setPixmap(*icon);
	
	mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
	
	QPushButton* okButton = new QPushButton("OK", this);
	okButton->setFixedWidth(100);
	okButton->setMaximumWidth(100);
	
	connect(okButton, SIGNAL(clicked()), this, SLOT(close()));
	
	// this alignment doesn't work with the size of the button limited, but...
	mainLayout->addWidget(okButton, Qt::AlignHCenter);
	
	mainLayout->addStretch(5);
}

