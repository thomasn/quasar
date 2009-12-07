// $Id: line_edit.cpp,v 1.34 2004/06/14 23:08:13 bpepers Exp $
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

#include "line_edit.h"
#include <qapplication.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qstyle.h>

LineEdit::LineEdit(QWidget* parent, const char* name)
    : QLineEdit(parent, name), _sizeChar('x'), _sizeLength(17)
{
    _changed = false;
    _valid = true;

    connect(this, SIGNAL(returnPressed()), SLOT(returnValidate()));
    connect(this, SIGNAL(textChanged(const QString&)), SLOT(markChanged()));
    connect(this, SIGNAL(doPopup(QKeySequence)), SLOT(popup(QKeySequence)));
}

LineEdit::LineEdit(int length, QWidget* parent, const char* name)
    : QLineEdit(parent, name), _sizeChar('x'), _sizeLength(length)
{
    _changed = false;
    _valid = true;

    connect(this, SIGNAL(returnPressed()), SLOT(returnValidate()));
    connect(this, SIGNAL(textChanged(const QString&)), SLOT(markChanged()));
    connect(this, SIGNAL(doPopup(QKeySequence)), SLOT(popup(QKeySequence)));

    setLength(length);
}

LineEdit::~LineEdit()
{
}

QSize
LineEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm(font());
    int h = fm.lineSpacing();
    int w = fm.width(_sizeChar) * _sizeLength;
    return QSize(w + 2 + 2 * frameWidth(), QMAX(h, 14) + 2 + 2 * frameWidth()).expandedTo(QApplication::globalStrut());
}

void
LineEdit::setSizeInfo(int length, char ch)
{
    _sizeLength = length;
    _sizeChar = ch;
    updateGeometry();
}

Variant
LineEdit::value()
{
    return text();
}

bool
LineEdit::valid()
{
    validate(Return);
    return _valid;
}

void
LineEdit::setValue(Variant value)
{
    switch (value.type()) {
    case Variant::T_NULL:
	setText("");
	break;
    case Variant::STRING:
	setText(value.toString());
	break;
    default:
	qWarning("Invalid type for LineEdit: %s", value.typeName());
	setText(value.toString());
    }
}

void
LineEdit::setText(const QString& val)
{
    QLineEdit::setText(val);
}

void
LineEdit::setLength(int length, char forWidth)
{
    int width = (length + 1) * fontMetrics().width(forWidth);
    setMinimumWidth(width);
    setMaximumWidth(width);
    setMaxLength(length);
}

void
LineEdit::setMinCharWidth(int characters, char forWidth)
{
    int width = (characters + 1) * fontMetrics().width(forWidth);
    setMinimumWidth(width);
}

bool
LineEdit::validate(Reason reason)
{
    if (!_changed) {
	if (!_valid) QApplication::beep();
	return _valid;
    }

    if (text().simplifyWhiteSpace() == "")
	_valid = true;
    else
	_valid = doValidation(reason);

    if (_valid) {
	unsetPalette();
	_changed = false;
	emit validData();
    } else {
	QApplication::beep();
	setPalette(QPalette(QColor(255, 20, 20)));
    }

    return _valid;
}

void
LineEdit::drawContents(QPainter* p)
{
    QLineEdit::drawContents(p);

    if (_pkeys.size() != 0 && hasFocus()) {
	QPointArray points(3);
	points[0] = QPoint(width() - 10, 0);
	points[1] = QPoint(width(), 0);
	points[2] = QPoint(width(), 10);

	p->setPen(tr("blue"));
	p->setBrush(tr("blue"));
	p->drawPolygon(points);
    }
}

void
LineEdit::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);
    if (_pkeys.size() == 0) return;

    QWidget* top = topLevelWidget();
    if (top->inherits("QMainWindow")) {
        QString message = longPopupText();
	((QMainWindow*)top)->statusBar()->message(message);
    } else {
	QString caption = topLevelWidget()->caption();
        QString message = shortPopupText();
	if (!caption.endsWith(message))
	    top->setCaption(top->caption() + message);
    }
}

void
LineEdit::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);
    if (_pkeys.size() == 0) return;

    QWidget* top = topLevelWidget();
    if (top->inherits("QMainWindow")) {
	((QMainWindow*)top)->statusBar()->clear();
    } else {
	QString caption = topLevelWidget()->caption();
	QString message = shortPopupText();
	int len = message.length();
	if (caption.endsWith(message))
	    topLevelWidget()->setCaption(caption.left(caption.length() - len));
    }
}

void
LineEdit::addPopup(const QKeySequence& key, const QString& text)
{
    _pkeys.push_back(key);
    _ptext.push_back(text);
}

QString
LineEdit::longPopupText() const
{
    if (_pkeys.size() == 0) return "";

    QString msg = tr("Press ");
    for (unsigned int i = 0; i < _pkeys.size(); ++i) {
        msg += QString(_pkeys[i]) + tr(" for ") + _ptext[i];
	if (i != _pkeys.size() - 1) msg += ", ";
    }
    return msg;
}

QString
LineEdit::shortPopupText() const
{
    if (_pkeys.size() == 0) return "";

    QString msg = " (";
    for (unsigned int i = 0; i < _pkeys.size(); ++i) {
        msg += QString(_pkeys[i]) + tr(" for ") + _ptext[i];
	if (i != _pkeys.size() - 1) msg += ",";
    }
    msg += ")";
    return msg;
}

QPopupMenu*
LineEdit::createPopupMenu()
{
    QPopupMenu* menu = QLineEdit::createPopupMenu();
    if (_pkeys.size() == 0) return menu;

    menu->insertSeparator();
    for (unsigned int i = 0; i < _pkeys.size(); ++i) {
        QString text = _ptext[i];
        QString msg = text.left(1).upper() + text.mid(1) + "...\t" +
	    QString(_pkeys[i]);

	menu->insertItem(msg, this, SLOT(popupChoice(int)), 0, i);
    }

    return menu;
}

bool
LineEdit::popupChoice(int id)
{
    if (id < 0 || id >= int(_pkeys.size()))
        return true;
    return popup(_pkeys[id]);
}

bool
LineEdit::popup(QKeySequence)
{
    return true;
}

bool
LineEdit::doValidation(Reason)
{
    return true;
}

bool
LineEdit::event(QEvent* e)
{
    QKeyEvent* k = (QKeyEvent*)e;
    QFocusEvent* f = (QFocusEvent*)e;

    switch (e->type()) {
    case QEvent::KeyPress:
        if (k->key() == Key_Tab || k->key() == Key_Backtab) {
	    if (!validate(Tab)) return true;
	} else {
	    for (unsigned int i = 0; i < _pkeys.size(); ++i) {
	        if (_pkeys[i] == QKeySequence(k->key())) {
		    emit doPopup(_pkeys[i]);
		    return true;
		}
	    }
	}
	break;
    case QEvent::FocusOut:
	if (f->reason() != QFocusEvent::ActiveWindow && f->reason() !=
		QFocusEvent::Popup)
	    validate(FocusOut);
	break;
    default:
	break;
    }

    return QLineEdit::event(e);
}

void
LineEdit::returnValidate()
{
    validate(Return);
}

void
LineEdit::markChanged()
{
    _changed = true;
}
