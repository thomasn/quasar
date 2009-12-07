// $Id: line_edit.h,v 1.22 2004/06/14 23:08:12 bpepers Exp $
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

#ifndef LINE_EDIT_H
#define LINE_EDIT_H

#include <qlineedit.h>
#include <qkeysequence.h>
#include <qvaluevector.h>
#include "variant.h"

class LineEdit: public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget* parent, const char* name=0);
    LineEdit(int length, QWidget* parent, const char* name=0);
    virtual ~LineEdit();

    enum Reason { Return, SetText, Tab, FocusOut, Unknown };

    virtual Variant value();
    bool valid();

    QSize sizeHint() const;

signals:
    void validData();
    void doPopup(QKeySequence key);

public slots:
    virtual void setValue(Variant value);
    virtual void setText(const QString& val);
    virtual void setLength(int length, char forWidth='x');
    virtual void setMinCharWidth(int characters, char forWidth='x');
    virtual void setSizeInfo(int length, char ch='x');
    virtual bool validate(Reason reason=Unknown);
    virtual void addPopup(const QKeySequence& key, const QString& text);
    virtual bool popup(QKeySequence key);
    virtual void markChanged();

protected slots:
    virtual void returnValidate();
    virtual bool popupChoice(int id);

protected:
    virtual void drawContents(QPainter* painter);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
    virtual QString shortPopupText() const;
    virtual QString longPopupText() const;
    virtual QPopupMenu* createPopupMenu();
    virtual bool doValidation(Reason reason);
    virtual bool event(QEvent* e);

    bool _changed;
    bool _valid;
    char _sizeChar;
    int _sizeLength;
    QValueVector<QKeySequence> _pkeys;
    QValueVector<QString> _ptext;
};

#endif // LINE_EDIT_H
