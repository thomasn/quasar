// $Id: column_edit.h,v 1.3 2004/12/04 21:25:59 bpepers Exp $
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

#ifndef COLUMN_EDIT_H
#define COLUMN_EDIT_H

#include <qmainwindow.h>
#include "data_model.h"

class TableEdit;
class LineEdit;
class MultiLineEdit;
class ComboBox;
class QCheckBox;

class ColumnEdit: public QMainWindow {
    Q_OBJECT
public:
    ColumnEdit(TableEdit* edit, const ColumnDefn& column);
    virtual ~ColumnEdit();

protected slots:
    bool slotOk();
    void slotNext();
    void slotReset();
    void slotCancel();
    void slotTypeChanged();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    void setWidgets();
    void getWidgets();
    bool checkChanged();

    TableEdit* _edit;		// Parent table edit window

    ColumnDefn _orig;		// Original column
    ColumnDefn _curr;		// Current edited column

    LineEdit* _name;
    MultiLineEdit* _desc;
    ComboBox* _type;
    LineEdit* _length;
    QCheckBox* _manditory;
    QCheckBox* _unique;
};

#endif // COLUMN_EDIT_H
