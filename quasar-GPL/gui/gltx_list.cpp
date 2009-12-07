// $Id: gltx_list.cpp,v 1.21 2004/01/31 01:50:30 arandell Exp $
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

#include "gltx_list.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx_select.h"
#include "date_valcon.h"
#include "general_master.h"
#include "cheque_master.h"

#include <assert.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlistview.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qpushbutton.h>

GltxList::GltxList(MainWindow* main)
    : ListWindow(main, "GltxList")
{
    _helpSource = "gltx_list.html";

    _list->addColumn(tr("Date"), fontMetrics().width("30 September 2000_"));
    _list->addColumn(tr("Memo"), fontMetrics().width("x") * 20);

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    menuBar()->removeItemAt(2);

    setCaption(tr("GL Transaction List"));
    finalize();
}

GltxList::~GltxList()
{
}

void
GltxList::dataEvent(DataEvent*)
{
}

void
GltxList::performRefresh()
{
    Id gltx_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, GltxSelect());
    DateValcon dateValcon;

    for (unsigned int i = 0; i < gltxs.size(); i++) {
	Gltx& gltx = gltxs[i];

	ListViewItem* lvi = new ListViewItem(_list, gltx.id());
	lvi->setText(0, dateValcon.format(gltx.postDate()));
	lvi->setText(1, gltx.memo());

	if (gltx.id() == gltx_id)
	    current = lvi;
    }

    if (current == NULL)
	current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
GltxList::performPrint()
{
    // TODO: print gltx list
}

void
GltxList::performNew()
{
    GeneralMaster* master = new GeneralMaster(_main, INVALID_ID);
    master->show();
}

void
GltxList::performEdit()
{
    Id gltx_id = currentId();
    Gltx gltx;
    _quasar->db()->lookup(gltx_id, gltx);

    QWidget* master = NULL;
    switch (gltx.dataType()) {
    case DataObject::CHEQUE:
	master = new ChequeMaster(_main, gltx_id);
	break;
    case DataObject::GENERAL:
	master = new GeneralMaster(_main, gltx_id);
	break;
    default:
	assert(false);
    }
    master->show();
}
