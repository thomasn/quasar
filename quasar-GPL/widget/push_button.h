// $Id: push_button.h,v 1.3 2004/02/03 00:56:03 arandell Exp $
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

#ifndef PUSH_BUTTON_H
#define PUSH_BUTTON_H

#include <qpushbutton.h>

class PushButton: public QPushButton {
    Q_OBJECT
public:
    PushButton(QWidget* parent, const char* name=0);
    PushButton(const QString& label, QWidget* parent, const char* name=0);
    virtual ~PushButton();

    virtual QSize sizeHint() const;

protected:
    int _widthPad;
    int _heightPad;
};

#endif // PUSH_BUTTON_H
