// $Id: print_stmt.h,v 1.5 2004/01/31 01:50:31 arandell Exp $
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

#ifndef PRINT_STMT_H
#define PRINT_STMT_H

#include "quasar_window.h"

class LookupEdit;
class LineEdit;
class QRadioButton;

class PrintStmt: public QuasarWindow {
    Q_OBJECT
public:
    PrintStmt(MainWindow* main);
    ~PrintStmt();

protected slots:
    void slotShowChanged(int button);
    void slotOk();
    void slotCancel();

protected:
    QRadioButton* _showAll;
    QRadioButton* _showGroup;
    QRadioButton* _showOne;
    LookupEdit* _group;
    LookupEdit* _customer;
    LookupEdit* _store;
};

#endif // PRINT_STMT_H
