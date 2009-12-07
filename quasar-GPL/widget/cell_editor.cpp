// $Id: cell_editor.cpp,v 1.24 2004/02/03 00:56:02 arandell Exp $
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

#include "cell_editor.h"
#include "line_edit.h"
#include "number_edit.h"
#include "check_box.h"
#include "date_popup.h"
#include "lookup_popup.h"
#include "lookup_edit.h"
#include "price_edit.h"
#include "table.h"

#include <qcombobox.h>

CellEditor::CellEditor(Table* table, int col, QWidget* widget)
{
    _table = table;
    _widget = widget;

    _widget->reparent(_table->viewport(), 0, QPoint());
    _table->addChild(_widget);

    _table->column(col)->editor = this;
    _widget->installEventFilter(table);
    hide();
}

CellEditor::~CellEditor()
{
    _table->removeChild(_widget);
    _widget->removeEventFilter(_table);
}

QWidget*
CellEditor::widget()
{
    return _widget;
}

bool
CellEditor::valid()
{
    return true;
}

void
CellEditor::show()
{
    _widget->show();
}

void
CellEditor::hide()
{
    _widget->hide();
}

void
CellEditor::setFocus()
{
    _widget->setFocus();
    if (_widget->inherits("QLineEdit"))
	((QLineEdit*)_widget)->selectAll();
}

void
CellEditor::resize(int width, int height)
{
    _widget->resize(width, height);
}

void
CellEditor::move(int x, int y)
{
    _table->moveChild(_widget, x, y);
}

LineEditor::LineEditor(Table* table, int col, LineEdit* widget)
    : CellEditor(table, col, widget)
{
}

LineEditor::~LineEditor()
{
}

LineEdit*
LineEditor::widget()
{
    return (LineEdit*)_widget;
}

Variant
LineEditor::value()
{
    return widget()->value();
}

void
LineEditor::setValue(Variant value)
{
    widget()->setValue(value);
}

bool
LineEditor::valid()
{
    return widget()->validate();
}

NumberEditor::NumberEditor(Table* table, int col, NumberEdit* widget)
    : LineEditor(table, col, widget)
{
}

NumberEditor::~NumberEditor()
{
}

NumberEdit*
NumberEditor::widget()
{
    return (NumberEdit*)_widget;
}

Variant
NumberEditor::value()
{
    return widget()->value();
}

void
NumberEditor::setValue(Variant value)
{
    widget()->setValue(value);
}

ComboEditor::ComboEditor(Table* table, int col, QComboBox* widget)
    : CellEditor(table, col, widget)
{
    widget->setBackgroundMode(QWidget::PaletteBase);
}

ComboEditor::~ComboEditor()
{
}

QComboBox*
ComboEditor::widget()
{
    return (QComboBox*)_widget;
}

Variant
ComboEditor::value()
{
    return widget()->currentText();
}

void
ComboEditor::setValue(Variant value)
{
    int i;

    switch (value.type()) {
    case Variant::T_NULL:
	widget()->setCurrentItem(0);
	break;
    case Variant::STRING:
	for (i = 0; i < widget()->count(); ++i)
	    if (widget()->text(i) == value.toString())
		widget()->setCurrentItem(i);
	break;
    case Variant::FIXED:
	widget()->setCurrentItem(value.toFixed().toInt());
	break;
    default:
	qWarning("Error: invalid value type for ComboEditor: %s",
		 value.typeName());
    }
}

CheckEditor::CheckEditor(Table* table, int col, CheckBox* widget)
    : CellEditor(table, col, widget)
{
    widget->setBackgroundMode(QWidget::PaletteBase);
}

CheckEditor::~CheckEditor()
{
}

CheckBox*
CheckEditor::widget()
{
    return (CheckBox*)_widget;
}

Variant
CheckEditor::value()
{
    return widget()->isChecked();
}

void
CheckEditor::setValue(Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:
	widget()->setChecked(false);
	break;
    case Variant::BOOL:
    case Variant::FIXED:
	widget()->setChecked(value.toBool());
	break;
    default:
	qWarning("Error: invalid value type for CheckEditor: %s",
		 value.typeName());
    }
}

DateEditor::DateEditor(Table* table, int col, DatePopup* widget)
    : CellEditor(table, col, widget)
{
}

DateEditor::~DateEditor()
{
}

DatePopup*
DateEditor::widget()
{
    return (DatePopup*)_widget;
}

Variant
DateEditor::value()
{
    return widget()->value();
}

void
DateEditor::setValue(Variant value)
{
    widget()->setValue(value);
}

bool
DateEditor::valid()
{
    return widget()->dateWidget()->validate();
}

PriceEditor::PriceEditor(Table* table, int col, PriceEdit* widget)
    : CellEditor(table, col, widget)
{
}

PriceEditor::~PriceEditor()
{
}

PriceEdit*
PriceEditor::widget()
{
    return (PriceEdit*)_widget;
}

Variant
PriceEditor::value()
{
    return widget()->getPrice();
}

void
PriceEditor::setValue(Variant value)
{
    widget()->setPrice(value.toPrice());
}

LookupEditor::LookupEditor(Table* table, int col, LookupEdit* widget)
    : CellEditor(table, col, widget)
{
    widget->removeEventFilter(table);
    widget->installEventFilter(table);
}

LookupEditor::~LookupEditor()
{
    widget()->removeEventFilter(_table);
}

LookupEdit*
LookupEditor::widget()
{
    return (LookupEdit*)_widget;
}

Variant
LookupEditor::value()
{
    return widget()->value();
}

void
LookupEditor::setValue(Variant value)
{
    widget()->setValue(value);
}

bool
LookupEditor::valid()
{
    return widget()->validate();
}

void
LookupEditor::setFocus()
{
    CellEditor::setFocus();
    widget()->selectAll();
}
