// $Id: cell_editor.h,v 1.20 2004/02/03 00:56:02 arandell Exp $
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

#ifndef CELL_EDITOR_H
#define CELL_EDITOR_H

#include "variant.h"

class Table;
class LineEdit;
class NumberEdit;
class CheckBox;
class DatePopup;
class PriceEdit;
class LookupEdit;
class QWidget;
class QComboBox;

class CellEditor {
public:
    CellEditor(Table* table, int col, QWidget* widget);
    virtual ~CellEditor();

    QWidget* widget();

    virtual Variant value() = 0;
    virtual void setValue(Variant value) = 0;
    virtual bool valid();

    virtual void show();
    virtual void hide();
    virtual void setFocus();
    virtual void resize(int width, int height);
    virtual void move(int x, int y);

protected:
    Table* _table;
    QWidget* _widget;
};

class LineEditor: public CellEditor {
public:
    LineEditor(Table* table, int col, LineEdit* widget);
    virtual ~LineEditor();

    LineEdit* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
    virtual bool valid();
};

class NumberEditor: public LineEditor {
public:
    NumberEditor(Table* table, int col, NumberEdit* widget);
    virtual ~NumberEditor();

    NumberEdit* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
};

class ComboEditor: public CellEditor {
public:
    ComboEditor(Table* table, int col, QComboBox* widget);
    virtual ~ComboEditor();

    QComboBox* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
};

class CheckEditor: public CellEditor {
public:
    CheckEditor(Table* table, int col, CheckBox* widget);
    virtual ~CheckEditor();

    CheckBox* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
};

class DateEditor: public CellEditor {
public:
    DateEditor(Table* table, int col, DatePopup* widget);
    virtual ~DateEditor();

    DatePopup* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
    virtual bool valid();
};

class PriceEditor: public CellEditor {
public:
    PriceEditor(Table* table, int col, PriceEdit* widget);
    virtual ~PriceEditor();

    PriceEdit* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
};

class LookupEditor: public CellEditor {
public:
    LookupEditor(Table* table, int col, LookupEdit* widget);
    virtual ~LookupEditor();

    LookupEdit* widget();

    virtual Variant value();
    virtual void setValue(Variant value);
    virtual bool valid();
    virtual void setFocus();
};

#endif
