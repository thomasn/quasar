// $Id: journal_window.cpp,v 1.19 2004/01/31 01:50:30 arandell Exp $
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

#include "journal_window.h"
#include "list_view.h"

#include <qlabel.h>
#include <qheader.h>
#include <qlayout.h>

JournalWindow::JournalWindow(QWidget* parent, const char* name)
    : QFrame(parent, name)
{
    list = new ListView(this);
    list->addColumn(tr("Type"), fontMetrics().width("Tender Adjustment__"));
    list->addTextColumn(tr("Number"), 10, AlignRight);
    list->addTextColumn(tr(" Store "), 8, AlignRight);
    list->addDateColumn(tr("Date"));
    list->addColumn(tr("Account"), fontMetrics().width("x") * 20);
    list->addMoneyColumn(tr("Debit"));
    list->addMoneyColumn(tr("Credit"));
    list->addCheckColumn(tr("Voided?"));
    list->setSorting(-1);
    list->setAllColumnsShowFocus(true);
    list->header()->setClickEnabled(false);
    list->setVScrollBarMode(QScrollView::AlwaysOn);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(list);

    needsRefresh = true;
    last = NULL;
}

JournalWindow::~JournalWindow()
{
}

void
JournalWindow::clearList()
{
    list->clear();
    last = NULL;
}
