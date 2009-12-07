// $Id: popup.cpp,v 1.17 2004/02/03 00:56:02 arandell Exp $
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

#include "popup.h"
#include "line_edit.h"

#include <qlayout.h>
#include <qlabel.h>

Popup::Popup(QWidget* text, QButton* button, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    _text = text;
    _button = button;
    _popup = NULL;

    text->reparent(this, 0, QPoint());
    if (text->inherits("LineEdit"))
	connect(text, SIGNAL(validData()), SIGNAL(validData()));

    button->reparent(this, 0, QPoint());
    connect(button, SIGNAL(clicked()), SLOT(openPopup()));

    if (topLevelWidget())
	topLevelWidget()->installEventFilter(this);
    text->installEventFilter(this);

    setFocusProxy(text);
    setFocusPolicy(StrongFocus);

    QGridLayout* grid = new QGridLayout(this);
    grid->setColStretch(0, 1);
    grid->addWidget(text, 0, 0);
    grid->addWidget(button, 0, 1, AlignLeft | AlignVCenter);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

QString
Popup::text() const
{
    QString text;

    if (_text->inherits("QLineEdit"))
	text = ((QLineEdit*)_text)->text();
    else if (_text->inherits("QLabel"))
	text = ((QLabel*)_text)->text();

    return text;
}

void
Popup::setText(const QString& string)
{
    if (_text->inherits("QLineEdit"))
	((QLineEdit*)_text)->setText(string);
    else if (_text->inherits("QLabel"))
	((QLabel*)_text)->setText(string);
}

QWidget*
Popup::popupWidget() const
{
    return _popup;
}

void
Popup::setPopupWidget(QWidget* popup)
{
    _popup = popup;
    if (_popup != NULL && !_popup->inherits("QDialog")) {
	QPoint point = mapToGlobal(QPoint(0, height()));
	_popup->move(point);
    }
}

void
Popup::setLength(int length)
{
    int width = length * _text->fontMetrics().width('x');
    _text->setMinimumWidth(width);
    if (_text->inherits("LineEdit")) {
	LineEdit* edit = (LineEdit*)_text;
	edit->setLength(length);
    } else if (_text->inherits("QLineEdit")) {
	QLineEdit* edit = (QLineEdit*)_text;
	edit->setMaxLength(length);
    }
    setMinimumWidth(sizeHint().width());
}

void
Popup::openPopup()
{
    if (_popup == NULL) return;
    _popup->show();
}

void
Popup::closePopup()
{
    _popup->hide();
}

void
Popup::moveEvent(QMoveEvent* e)
{
    QWidget::moveEvent(e);
    setPopupWidget(_popup);
}

void
Popup::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    setPopupWidget(_popup);
}

bool
Popup::eventFilter(QObject* o, QEvent* e)
{
    if (o == _text) {
	if (e->type() == QEvent::KeyPress) {
	    QKeyEvent* ke = (QKeyEvent*)e;
	    if (ke->key() == Key_F9)
		openPopup();
	}
    } else {
	switch (e->type()) {
	case QEvent::Move:
	case QEvent::Resize:
	    setPopupWidget(_popup);
	    break;
	default:
	    break;
	}
    }

    return false;
}
