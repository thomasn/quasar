// $Id: table_column.h,v 1.13 2005/02/24 01:30:56 bpepers Exp $
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

#ifndef TABLE_COLUMN_H
#define TABLE_COLUMN_H

#include <qnamespace.h>
#include <qstring.h>
#include <qstringlist.h>

#include "date_valcon.h"
#include "money_valcon.h"
#include "percent_valcon.h"
#include "price_valcon.h"

class Table;
class QPainter;
class CellEditor;
class LookupWindow;

class TableColumn {
public:
    TableColumn(Table* table, const QString& name, int width);
    virtual ~TableColumn();

    virtual void paint(int row, int col, QPainter* p) = 0;
    void checkWidth();

    Table* table;
    QString name;
    int width;
    int minWidth;
    int maxWidth;
    CellEditor* editor;
};

class TextColumn: public TableColumn {
public:
    TextColumn(Table* table, const QString& name, int chars,
	       int alignment=Qt::AlignLeft);

    virtual void paint(int row, int col, QPainter* p);

    int alignment;
};

class DateColumn: public TextColumn {
public:
    DateColumn(Table* table, const QString& name);

    virtual void paint(int row, int col, QPainter* p);

    DateValcon valcon;
};

class ComboColumn: public TextColumn {
public:
    ComboColumn(Table* table, const QString& name, int chars,
		int alignment=Qt::AlignLeft);

    virtual void paint(int row, int col, QPainter* p);

    QStringList strings;
};

class CheckColumn: public TableColumn {
public:
    CheckColumn(Table* table, const QString& name);

    virtual void paint(int row, int col, QPainter* p);
};

class NumberColumn: public TextColumn {
public:
    NumberColumn(Table* table, const QString& name, int digits=6);

    virtual void paint(int row, int col, QPainter* p);
};

class MoneyColumn: public NumberColumn {
public:
    MoneyColumn(Table* table, const QString& name, int digits=7);

    virtual void paint(int row, int col, QPainter* p);
};

class PercentColumn: public NumberColumn {
public:
    PercentColumn(Table* table, const QString& name, int digits=4);

    virtual void paint(int row, int col, QPainter* p);
};

class PriceColumn: public TextColumn {
public:
    PriceColumn(Table* table, const QString& name, int digits=8);

    virtual void paint(int row, int col, QPainter* p);

    PriceValcon valcon;
};

class LookupColumn: public TextColumn {
public:
    LookupColumn(Table* table, const QString& name, int chars,
		 LookupWindow* lookup);

    virtual void paint(int row, int col, QPainter* p);

    LookupWindow* lookup;
};

#endif
