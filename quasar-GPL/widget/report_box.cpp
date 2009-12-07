// $Id: report_box.cpp,v 1.10 2004/06/26 21:35:44 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding a valid Quasar Commercial License may use this file
// in accordance with the Quasar Commercial License Agreement provided
// with the Software in the LICENSE.COMMERCIAL file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "report_box.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qgrid.h>

ReportBox::ReportBox(QWidget* parent, const char* name)
    : QTabWidget(parent, name)
{
    QGrid* grid = new QGrid(REPORT_BOX_COLS, QGrid::Horizontal, this);
    addTab(grid, tr("Reports"));

    for (int i = 0; i < REPORT_BOX_CNT; ++i) {
	ReportLabel* label = new ReportLabel(" ", grid);
	connect(label, SIGNAL(clicked(ReportLabel*)),
		SLOT(labelClicked(ReportLabel*)));
	label->setBackgroundMode(QWidget::PaletteBase);
	_labels[i] = label;
    }
}

ReportBox::~ReportBox()
{
}

QString
ReportBox::text(int row, int col)
{
    int index = (row * REPORT_BOX_COLS) + col;
    if (index >= 0 && index < REPORT_BOX_CNT)
	return _labels[index]->text();
    return "";
}

void
ReportBox::setText(int row, int col, const QString& text)
{
    int index = (row * REPORT_BOX_COLS) + col;
    if (index >= 0 && index < REPORT_BOX_CNT)
	_labels[index]->setText(text);
}

QString
ReportBox::type(int row, int col)
{
    int index = (row * REPORT_BOX_COLS) + col;
    if (index >= 0 && index < REPORT_BOX_CNT)
	return _labels[index]->type;
    return "";
}

void
ReportBox::setType(int row, int col, const QString& type)
{
    int index = (row * REPORT_BOX_COLS) + col;
    if (index >= 0 && index < REPORT_BOX_CNT)
	_labels[index]->type = type;
}

QString
ReportBox::args(int row, int col)
{
    int index = (row * REPORT_BOX_COLS) + col;
    if (index >= 0 && index < REPORT_BOX_CNT)
	return _labels[index]->args;
    return "";
}

void
ReportBox::setArgs(int row, int col, const QString& args)
{
    int index = (row * REPORT_BOX_COLS) + col;
    if (index >= 0 && index < REPORT_BOX_CNT)
	_labels[index]->args = args;
}

void
ReportBox::labelClicked(ReportLabel* label)
{
    for (int row = 0; row < REPORT_BOX_ROWS; ++row) {
	for (int col = 0; col < REPORT_BOX_COLS; ++col) {
	    int index = (row * REPORT_BOX_COLS) + col;
	    if (_labels[index] == label) {
		emit clicked(row, col);
		emit clicked(label->type, label->args);
	    }
	}
    }
}
