// $Id: inquiry_window.cpp,v 1.25 2004/01/31 01:50:30 arandell Exp $
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

#include "inquiry_window.h"
#include "main_window.h"
#include "date_popup.h"
#include "lookup_edit.h"
#include "list_view.h"
#include "grid.h"

#include <qlabel.h>
#include <qheader.h>
#include <qlayout.h>

InquiryWindow::InquiryWindow(MainWindow* _main, QWidget* parent,
			     const char* name, LookupWindow* lookup)
    : QFrame(parent, name), main(_main), quasar(_main->quasar())
{
    QFrame* top = new QFrame(this);
    QFrame* bot = new QFrame(this);

    QLabel* searchLabel = new QLabel(tr("Search:"), top);
    search = new LookupEdit(lookup, top);
    searchLabel->setBuddy(search);

    QGridLayout* grid = new QGridLayout(top);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addWidget(searchLabel, 0, 0);
    grid->addWidget(search, 0, 1, AlignLeft | AlignVCenter);

    list = new ListView(this);
    list->addColumn(tr("Type"), fontMetrics().width("Tender Adjustment__"));
    list->addTextColumn(tr("Number"), 10, AlignRight);
    list->addTextColumn(tr("Store"), 6, AlignRight);
    list->addDateColumn(tr("Date"));
    list->addTextColumn(tr("Memo"), 20);
    list->addMoneyColumn(tr("Debit"));
    list->addMoneyColumn(tr("Credit"));
    list->addCheckColumn(tr("Voided?"));
    list->setSorting(2);
    list->setAllColumnsShowFocus(true);
    list->setShowSortIndicator(true);
    list->header()->setClickEnabled(true);
    list->setVScrollBarMode(QScrollView::AlwaysOn);

    // Create bottom grid of 5x2 labels
    QGridLayout* grid2 = new QGridLayout(bot);
    grid2->setSpacing(6);
    grid2->setMargin(6);
    for (int row = 0; row < 2; ++row) {
	for (int col = 0; col < 5; ++col) {
	    QLabel* label = new QLabel(" ", bot);
	    label->setAlignment(AlignRight | AlignVCenter);
	    grid2->addWidget(label, row, col);
	    labels[row * 5 + col] = label;
	}
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(top);
    layout->addWidget(list);
    layout->addWidget(bot);

    _grid = NULL;
    needsRefresh = false;
    loading = false;
    //search->setFocus();
}

InquiryWindow::~InquiryWindow()
{
}

void
InquiryWindow::slotPrint()
{
    if (_grid != NULL)
	_grid->print(this);
}
