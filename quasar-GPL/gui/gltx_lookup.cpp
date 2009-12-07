// $Id: gltx_lookup.cpp,v 1.11 2004/09/28 16:45:46 bpepers Exp $
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

#include "gltx_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "gltx.h"
#include "gltx_select.h"
#include "list_view_item.h"
#include "line_edit.h"
#include "screen_decl.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>

GltxLookup::GltxLookup(MainWindow* main, QWidget* parent, int matchType)
    : QuasarLookup(main, parent, "GltxLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14);
    _list->addTextColumn(tr("Type"), 20);
    _list->addDateColumn(tr("Date"));
    setCaption(tr("Transaction Lookup"));
    setAllowNew(false);

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new TxCombo(_quasar, _search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->setDataType(matchType);
}

GltxLookup::GltxLookup(QuasarClient* quasar, QWidget* parent, int matchType)
    : QuasarLookup(quasar, parent, "GltxLookup"), activeOnly(true)
{
    _label->setText(tr("Number:"));
    _text->setLength(14);
    _list->addTextColumn(tr("Number"), 14);
    _list->addTextColumn(tr("Type"), 20);
    _list->addDateColumn(tr("Date"));
    setCaption(tr("Transaction Lookup"));
    setAllowNew(false);

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new TxCombo(_quasar, _search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->setDataType(matchType);
}

GltxLookup::~GltxLookup()
{
}

QString
GltxLookup::lookupById(Id gltx_id)
{
    Gltx gltx;
    if (gltx_id != INVALID_ID && _quasar->db()->lookup(gltx_id, gltx))
	return gltx.number();
    return "";
}

vector<DataPair>
GltxLookup::lookupByText(const QString& text)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    GltxSelect conditions;
    if (!text.isEmpty())
	conditions.number = text;
    conditions.type = type->dataType();
    conditions.activeOnly = activeOnly;

    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    vector<DataPair> data;
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	data.push_back(DataPair(gltxs[i].id(), gltxs[i].number()));
    }

    QApplication::restoreOverrideCursor();
    return data;
}

void
GltxLookup::refresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _list->clear();
    GltxSelect conditions;
    if (!text().isEmpty())
	conditions.number = text() + "%";
    conditions.type = type->dataType();
    conditions.activeOnly = activeOnly;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	QApplication::restoreOverrideCursor();

	QWidget* parent = this;
	if (isHidden() && parentWidget() != NULL)
	    parent = parentWidget();

	QString message = tr("This will select %1 items\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(parent, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	Gltx& gltx = gltxs[i];

	ListViewItem* item = new ListViewItem(_list, gltx.id());
	item->setValue(0, gltx.number());
	item->setValue(1, gltx.dataTypeName());
	item->setValue(2, gltx.postDate());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);

    QApplication::restoreOverrideCursor();
}

QWidget*
GltxLookup::slotNew()
{
    return NULL;
}

QWidget*
GltxLookup::slotEdit(Id id)
{
    Gltx gltx;
    if (!_quasar->db()->lookup(id, gltx)) return NULL;

    QWidget* master = editGltx(gltx, _main);
    if (master == NULL) return NULL;

    master->show();
    reject();
    return master;
}
