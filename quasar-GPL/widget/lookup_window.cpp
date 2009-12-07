// $Id: lookup_window.cpp,v 1.22 2004/03/20 01:33:45 bpepers Exp $
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

#include "lookup_window.h"
#include "list_view_item.h"
#include "line_edit.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

LookupWindow::LookupWindow(QWidget* parent, const char* name)
    : QDialog(parent, name, true), _autoRefresh(true), _ignoreChanges(false)
{
    _search = new QFrame(this);
    _label = new QLabel(tr("Look for:"), _search);
    _text = new LineEdit(_search);
    _text->installEventFilter(this);
    _refresh = new QPushButton(tr("&Refresh"), _search);
    _refresh->setFocusPolicy(NoFocus);
    connect(_refresh, SIGNAL(clicked()), SLOT(refresh()));

    _list = new ListView(this);
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    _list->setVScrollBarMode(QScrollView::AlwaysOn);

    _buttons = new QFrame(this);
    _new = new QPushButton(tr("&New"), _buttons);
    _edit = new QPushButton(tr("&Edit"), _buttons);
    _ok = new QPushButton(tr("&OK"), _buttons);
    _ok->setDefault(true);
    _cancel = new QPushButton(tr("&Cancel"), _buttons);

    _searchGrid = new QGridLayout(_search);
    _searchGrid->setSpacing(3);
    _searchGrid->setMargin(3);
    _searchGrid->setColStretch(8, 1);
    _searchGrid->addColSpacing(8, 10);
    _searchGrid->addWidget(_label, 0, 0);
    _searchGrid->addWidget(_text, 0, 1, AlignLeft | AlignCenter);
    _searchGrid->addWidget(_refresh, 0, 9);

    _buttonGrid = new QGridLayout(_buttons);
    _buttonGrid->setSpacing(3);
    _buttonGrid->setMargin(3);
    _buttonGrid->setRowStretch(5, 1);
    _buttonGrid->addColSpacing(5, 10);
    _buttonGrid->addWidget(_new, 0, 0);
    _buttonGrid->addWidget(_edit, 0, 1);
    _buttonGrid->addWidget(_ok, 0, 8);
    _buttonGrid->addWidget(_cancel, 0, 9);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->addWidget(_search, 0, 0);
    grid->addWidget(_list, 1, 0);
    grid->addWidget(_buttons, 2, 0);

    connect(_new, SIGNAL(clicked()), SLOT(slotNew()));
    connect(_edit, SIGNAL(clicked()), SLOT(slotEdit()));
    connect(_ok, SIGNAL(clicked()), SLOT(accept()));
    connect(_cancel, SIGNAL(clicked()), SLOT(reject()));
    connect(_list, SIGNAL(doubleClicked(QListViewItem*)), SLOT(accept()));
    connect(_list, SIGNAL(currentChanged(QListViewItem*)),
	    SLOT(currentChanged(QListViewItem*)));
    connect(_list, SIGNAL(clicked(QListViewItem*)),
	    SLOT(currentChanged(QListViewItem*)));
}

LookupWindow::~LookupWindow()
{
}

void
LookupWindow::clearLines()
{
    _list->clear();
}

bool
LookupWindow::autoRefresh() const
{
    return _autoRefresh;
}

bool
LookupWindow::allowNew() const
{
    return _new->isEnabled();
}

bool
LookupWindow::allowEdit() const
{
    return _edit->isEnabled();
}

void
LookupWindow::setAutoRefresh(bool flag)
{
    _autoRefresh = flag;
}

void
LookupWindow::setAllowNew(bool flag)
{
    _new->setEnabled(flag);
}

void
LookupWindow::setAllowEdit(bool flag)
{
    _edit->setEnabled(flag);
}

QString
LookupWindow::text() const
{
    return _text->text();
}

int
LookupWindow::matchCount(const QString& text)
{
    vector<DataPair> data = lookupByText(text);
    return data.size();
}

Id
LookupWindow::getId() const
{
    ListViewItem* item = (ListViewItem*)_list->currentItem();
    if (item == NULL) return INVALID_ID;
    return item->id;
}

void
LookupWindow::setText(const QString& text)
{
    if (_ignoreChanges) return;
    _ignoreChanges = true;

    _text->setText(text);
    _text->setFocus();

    _ignoreChanges = false;
}

void
LookupWindow::currentChanged(QListViewItem* item)
{
    if (_ignoreChanges) return;
    _ignoreChanges = true;

    ensureItemVisible(item);

    _ignoreChanges = false;
}

void
LookupWindow::resizeEvent(QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    ensureItemVisible(_list->currentItem());
}

void
LookupWindow::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
    ensureItemVisible(_list->currentItem());
}

bool
LookupWindow::eventFilter(QObject* o, QEvent* e)
{
    if (o == _text && e->type() == QEvent::KeyPress) {
	QKeyEvent* ke = (QKeyEvent*)e;

	switch (ke->key()) {
	case Key_Up:	qApp->sendEvent(_list, e); break;
	case Key_Down:	qApp->sendEvent(_list, e); break;
	case Key_Next:	qApp->sendEvent(_list, e); break;
	case Key_Prior:	qApp->sendEvent(_list, e); break;
	case Key_Home:	qApp->sendEvent(_list, e); return true;
	case Key_End:	qApp->sendEvent(_list, e); return true;
	}
    }

    return QDialog::eventFilter(o, e);
}

void
LookupWindow::findClosest()
{
    QString input = _text->text().upper();
    QListViewItem* item = _list->firstChild();

    while (item != NULL) {
	QString text = item->text(0).upper();
	if (text >= input) break;
	item = item->nextSibling();
    }

    _list->setCurrentItem(item);
    if (item != NULL) {
	int y = item->itemPos();
	_list->ensureVisible(0, y, 0, _list->visibleHeight() / 2);
    }
}

void
LookupWindow::accept()
{
    if (_list->childCount() == 0) {
	reject();
	return;
    }

    QListViewItem* item = _list->currentItem();
    if (item != NULL)
	_text->setText(item->text(0));

    QDialog::accept();
}

void
LookupWindow::ensureItemVisible(QListViewItem* item)
{
    if (item == NULL) return;
    _list->ensureItemVisible(item);
}

void
LookupWindow::slotEdit()
{
    Id id = getId();
    if (id != INVALID_ID) slotEdit(id);
}
