// $Id: table_column.cpp,v 1.24 2005/02/24 01:30:56 bpepers Exp $
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

#include "table_column.h"
#include "table.h"
#include "table_row.h"
#include "lookup_window.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstring.h>
#include <math.h>

#define tr(text) qApp->translate("TableColumn", text)

TableColumn::TableColumn(Table* t, const QString& n, int w)
{
    table = t;
    name = n;
    width = w;
    minWidth = 0;
    maxWidth = 9999;
    editor = NULL;

    table->addColumn(this);
}

TableColumn::~TableColumn()
{
    delete editor;
}

void
TableColumn::checkWidth()
{
    int w = table->fontMetrics().width(name) + 32;
    if (width < w) width = w;
}

TextColumn::TextColumn(Table* t, const QString& n, int chars, int a)
    : TableColumn(t, n, 0)
{
    alignment = a;
    width = t->fontMetrics().width("x") * chars;
    checkWidth();
}

void
TextColumn::paint(int row, int col, QPainter* p)
{
    QString value = table->cellValue(row, col, false).toString();

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, value);
}

DateColumn::DateColumn(Table* t, const QString& n)
    : TextColumn(t, n, 0)
{
    QString text = valcon.format(QDate(2000, 9, 30));
    width = t->fontMetrics().width(text);
    checkWidth();
}

void
DateColumn::paint(int row, int col, QPainter* p)
{
    QString text = "";
    Variant value = table->cellValue(row, col);

    switch (value.type()) {
    case Variant::DATE:
	text = valcon.format(value.toDate());
	break;
    case Variant::STRING:
	if (value != "") {
	    valcon.parse(value.toString());
	    text = valcon.format();
	}
	break;
    case Variant::T_NULL:
	break;
    default:
	text = tr("Invalid date type: %1").arg(value.type());
    }

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}

ComboColumn::ComboColumn(Table* t, const QString& n, int w, int a)
    : TextColumn(t, n, w, a)
{
}

void
ComboColumn::paint(int row, int col, QPainter* p)
{
    Variant value = table->cellValue(row, col);
    QString text;

    switch (value.type()) {
    case Variant::T_NULL:
	text = "";
	break;
    case Variant::FIXED:
	text = strings[value.toFixed().toInt()];
	break;
    default:
	text = tr("Invalid combo type: %1").arg(value.type());
    }

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}

CheckColumn::CheckColumn(Table* t, const QString& n)
    : TableColumn(t, n, 0)
{
    checkWidth();
}

void
CheckColumn::paint(int row, int col, QPainter* p)
{
    bool value = table->cellValue(row, col, false).toBool();

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);
    if (w < 3 || h < 3) return;

    // Draw contents
    if (value) {
	QRect rect(1, 1, w - 2, h - 2);
	table->style().drawPrimitive(QStyle::PE_CheckMark, p, rect,
				     table->colorGroup());
    }
}

NumberColumn::NumberColumn(Table* t, const QString& n, int digits)
    : TextColumn(t, n, 0, Qt::AlignRight)
{
    fixed number = pow(10.0, digits) - 1.0;
    QString text = NumberValcon().format(number);
    width = t->fontMetrics().width(text);
    checkWidth();
}

void
NumberColumn::paint(int row, int col, QPainter* p)
{
    QString text = "";
    Variant value = table->cellValue(row, col);
    NumberValcon valcon;

    switch (value.type()) {
    case Variant::FIXED:
	text = valcon.format(value.toFixed());
	break;
    case Variant::STRING:
	if (value != "") {
	    valcon.parse(value.toString());
	    text = valcon.format();
	}
	break;
    case Variant::T_NULL:
	break;
    default:
	text = tr("Invalid number type: %1").arg(value.type());
    }

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}

MoneyColumn::MoneyColumn(Table* t, const QString& n, int digits)
    : NumberColumn(t, n, 0)
{
    fixed number = pow(10.0, digits) - 1.0;
    QString text = MoneyValcon().format(number);
    width = t->fontMetrics().width(text);
    checkWidth();
}

void
MoneyColumn::paint(int row, int col, QPainter* p)
{
    QString text = "";
    Variant value = table->cellValue(row, col);
    MoneyValcon valcon;

    switch (value.type()) {
    case Variant::FIXED:
	text = valcon.format(value.toFixed());
	break;
    case Variant::STRING:
	if (value != "") {
	    valcon.parse(value.toString());
	    text = valcon.format();
	}
	break;
    case Variant::T_NULL:
	break;
    default:
	text = tr("Invalid number type: %1").arg(value.type());
    }

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}

PercentColumn::PercentColumn(Table* t, const QString& n, int digits)
    : NumberColumn(t, n, 0)
{
    fixed number = pow(10.0, digits) - 1.0;
    QString text = PercentValcon().format(number);
    width = t->fontMetrics().width(text);
    checkWidth();
}

void
PercentColumn::paint(int row, int col, QPainter* p)
{
    QString text = "";
    Variant value = table->cellValue(row, col);
    PercentValcon valcon;

    switch (value.type()) {
    case Variant::FIXED:
	text = valcon.format(value.toFixed());
	break;
    case Variant::STRING:
	if (value != "") {
	    valcon.parse(value.toString());
	    text = valcon.format();
	}
	break;
    case Variant::T_NULL:
	break;
    default:
	text = tr("Invalid number type: %1").arg(value.type());
    }

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}

PriceColumn::PriceColumn(Table* t, const QString& n, int digits)
    : TextColumn(t, n, 0, Qt::AlignRight)
{
    fixed number = pow(10.0, digits) - 1.0;
    QString text = valcon.format(Price(number));
    width = t->fontMetrics().width(text);
    checkWidth();
}

void
PriceColumn::paint(int row, int col, QPainter* p)
{
    Variant value = table->cellValue(row, col);
    QString text = valcon.format(value.toPrice());

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}

LookupColumn::LookupColumn(Table* t, const QString& n, int w, LookupWindow* l)
    : TextColumn(t, n, w, Qt::AlignLeft)
{
    lookup = l;
}

void
LookupColumn::paint(int row, int col, QPainter* p)
{
    Variant value = table->cellValue(row, col);
    QString text;
    Id id;

    switch (value.type()) {
    case Variant::T_NULL:
	text = "";
	break;
    case Variant::ID:
	text = lookup->lookupById(value.toId());
	break;
    case Variant::PLU:
	text = value.toPlu().number();
	break;
    case Variant::STRING:
	text = value.toString();
	break;
    default:
	text = tr("Invalid lookup type: %1").arg(value.type());
    }

    int w = table->columnWidth(col);
    int h = table->rowHeight(row);

    // Draw contents
    p->drawText(1, 1, w - 2, h - 2, alignment | Qt::AlignVCenter, text);
}
