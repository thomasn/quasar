// $Id: list_view_item.cpp,v 1.21 2004/10/21 05:45:06 bpepers Exp $
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

#include "list_view_item.h"
#include "valcon.h"

#include <assert.h>
#include <qpainter.h>
#include <qstyle.h>

ListViewItem::ListViewItem(ListView* parent, Id _id)
    : QListViewItem(parent)
{
    id = _id;
    isLast = false;
    values.resize(listView()->columns());
}

ListViewItem::ListViewItem(ListView* parent, ListViewItem* after, Id _id)
    : QListViewItem(parent, after)
{
    id = _id;
    isLast = false;
    values.resize(listView()->columns());
}

ListViewItem::ListViewItem(ListViewItem* parent, Id _id)
    : QListViewItem(parent)
{
    id = _id;
    isLast = false;
    values.resize(listView()->columns());
}

ListViewItem::ListViewItem(ListViewItem* parent, ListViewItem* after, Id _id)
    : QListViewItem(parent, after)
{
    id = _id;
    isLast = false;
    values.resize(listView()->columns());
}

ListViewItem::~ListViewItem()
{
}

ListView*
ListViewItem::listView() const
{
    return (ListView*)QListViewItem::listView();
}

ListViewItem*
ListViewItem::firstChild() const
{
    return (ListViewItem*)QListViewItem::firstChild();
}

ListViewItem*
ListViewItem::nextSibling() const
{
    return (ListViewItem*)QListViewItem::nextSibling();
}

ListViewItem*
ListViewItem::parent() const
{
    return (ListViewItem*)QListViewItem::parent();
}

ListViewItem*
ListViewItem::itemAbove()
{
    return (ListViewItem*)QListViewItem::itemAbove();
}

ListViewItem*
ListViewItem::itemBelow()
{
    return (ListViewItem*)QListViewItem::itemBelow();
}

QString
ListViewItem::key(int column, bool ascending) const
{
#if 1
    return QListViewItem::key(column, ascending);
#else
    assert(column >= 0 && column < int(values.size()));
    Variant value = values[column];

    QString result;
    switch (value.type()) {
    case Variant::FIXED:
	if (value.toFixed().toDouble() < 0.0)
	    result.sprintf("m%015.4f", -value.toFixed().toDouble());
	else
	    result.sprintf("p%015.4f", value.toFixed().toDouble());
	break;
    case Variant::DATE:
    case Variant::TIME:
	result.sprintf("%010d", value.toFixed().toInt());
	break;
    default:
	result = value.toString();
	if (listView()->columnAlignment(column) == AlignRight)
	    result = result.sprintf("%20s", result.latin1());
    }
    qDebug(result);

    return result;
#endif
}

int
ListViewItem::compare(QListViewItem* i, int col, bool) const
{
    ListViewItem* item = (ListViewItem*)i;
    if (item->isLast) return -1;
    if (isLast) return 1;

    Variant lhs = value(col);
    Variant rhs = item->value(col);

    if (lhs.type() == rhs.type() && lhs.type() == Variant::STRING) {
	lhs = lhs.toString().lower();
	rhs = rhs.toString().lower();
	if (listView()->columnAlignment(col) == AlignRight) {
	    lhs = QString("%1").arg(lhs.toString(), 20);
	    rhs = QString("%1").arg(rhs.toString(), 20);
	}
    }

    return lhs.compare(rhs);
}

Variant
ListViewItem::value(int column) const
{
    assert(column >= 0 && column < int(values.size()));
    return values[column];
}

void
ListViewItem::setValue(int column, Variant value)
{
    assert(column >= 0 && column < int(values.size()));
    values[column] = value;

    Valcon* valcon = listView()->valcon(column);
    if (valcon == NULL)
	QListViewItem::setText(column, value.toString());
    else
	QListViewItem::setText(column, valcon->format(value));
}

void
ListViewItem::setText(int column, const QString& value)
{
    assert(column >= 0 && column < int(values.size()));
    values[column] = value;
    QListViewItem::setText(column, value);
}

void
ListViewItem::paintCell(QPainter* p, const QColorGroup& cg, int column,
			int width, int alignment)
{
    Variant v = value(column);
    if (v.type() != Variant::BOOL) {
	QListViewItem::paintCell(p, cg, column, width, alignment);
	return;
    }

    if (isSelected() && (column == 0 || listView()->allColumnsShowFocus()))
	p->fillRect(0, 0, width, height(), cg.highlight());
    else
	p->fillRect(0, 0, width, height(), cg.base());

    if (v.toBool()) {
	QRect rect(1, 1, width - 2, height() - 2);
	listView()->style().drawPrimitive(QStyle::PE_CheckMark, p, rect, cg);
    }
}
