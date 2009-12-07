// $Id: list_view.cpp,v 1.19 2005/01/30 04:40:58 bpepers Exp $
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

#include "list_view.h"
#include "money_valcon.h"
#include "percent_valcon.h"
#include "date_valcon.h"
#include "price_valcon.h"

#include <assert.h>
#include <math.h>
#include <qapplication.h>

ListView::ListView(QWidget* parent, const char* name, WFlags flags)
    : QListView(parent, name, flags)
{
}

ListView::~ListView()
{
    for (unsigned int i = 0; i < _valcons.size(); ++i)
	delete _valcons[i];
}

int
ListView::addColumn(const QString& label, int size, Valcon* valcon)
{
    if (size < fontMetrics().width(label + "__"))
	size = fontMetrics().width(label + "__");
    int column = QListView::addColumn(label, size);
    setValcon(column, valcon);
    return column;
}

int
ListView::addTextColumn(const QString& label, int length, int align)
{
    char ch = (align == AlignRight) ? '9' : 'x';
    int column = addColumn(label, qApp->fontMetrics().width(ch) * length + 5);
    setColumnAlignment(column, align);
    return column;
}

int
ListView::addNumberColumn(const QString& label, int digits)
{
    fixed value = pow(10.0, digits) - 1;
    QString text = NumberValcon().format(value);
    int column = addColumn(label, qApp->fontMetrics().width(text),
			   new NumberValcon());
    setColumnAlignment(column, AlignRight);
    return column;
}

int
ListView::addMoneyColumn(const QString& label, int digits)
{
    fixed value = pow(10.0, digits) - .01;
    QString widest = MoneyValcon().format(value) + "__";
    int column = addColumn(label, qApp->fontMetrics().width(widest),
			   new MoneyValcon());
    setColumnAlignment(column, AlignRight);
    return column;
}

int
ListView::addPercentColumn(const QString& label, int digits)
{
    fixed value = pow(10.0, digits) - .01;
    QString widest = PercentValcon().format(value) + "__";
    int column = addColumn(label, qApp->fontMetrics().width(widest),
			   new PercentValcon());
    setColumnAlignment(column, AlignRight);
    return column;
}

int
ListView::addDateColumn(const QString& label)
{
    QString widest = DateValcon().format(QDate(2000, 9, 30)) + "__";
    return addColumn(label, qApp->fontMetrics().width(widest),
		     new DateValcon());
}

int
ListView::addCheckColumn(const QString& label)
{
    int column = addColumn(label, qApp->fontMetrics().width(label + "_"));
    setColumnAlignment(column, AlignCenter);
    return column;
}

int
ListView::addPriceColumn(const QString& label)
{
    QString widest = MoneyValcon().format(fixed(99999999.99)) + "__";
    int column = addColumn(label, qApp->fontMetrics().width(widest),
			   new PriceValcon());
    setColumnAlignment(column, AlignRight);
    return column;
}

Valcon*
ListView::valcon(int column) const
{
    if (column < 0 || column >= columns())
	return NULL;
    return _valcons[column];
}

void
ListView::setValcon(int column, Valcon* valcon)
{
    assert (column >= 0 && column < columns());
    if (column >= (int)_valcons.size())
	_valcons.resize(columns());
    else
	delete _valcons[column];
    _valcons[column] = valcon;
}

int
ListView::findColumn(const QString& title) const
{
    for (int i = 0; i < columns(); ++i)
	if (columnText(i) == title) return i;
    return -1;
}

ListViewItem*
ListView::selectedItem() const
{
    return (ListViewItem*)QListView::selectedItem();
}

ListViewItem*
ListView::currentItem() const
{
    return (ListViewItem*)QListView::currentItem();
}

ListViewItem*
ListView::firstChild() const
{
    return (ListViewItem*)QListView::firstChild();
}
