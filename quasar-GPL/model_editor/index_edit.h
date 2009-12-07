// $Id: index_edit.h,v 1.1 2004/12/04 21:23:57 bpepers Exp $
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

#ifndef INDEX_EDIT_H
#define INDEX_EDIT_H

#include <qmainwindow.h>
#include "data_model.h"

class TableEdit;
class LineEdit;
class MultiLineEdit;
class ListView;
class QCheckBox;

class IndexEdit: public QMainWindow {
    Q_OBJECT
public:
    IndexEdit(TableEdit* edit, const IndexDefn& index);
    virtual ~IndexEdit();

protected slots:
    bool slotOk();
    void slotNext();
    void slotReset();
    void slotCancel();
    void slotAddColumn();
    void slotEditColumn();
    void slotRemoveColumn();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    void setWidgets();
    void getWidgets();
    void refreshColumns();
    bool checkChanged();
    IndexColumn* currentColumn();

    TableEdit* _edit;		// Parent table edit window

    IndexDefn _orig;		// Original index
    IndexDefn _curr;		// Current edited index

    LineEdit* _name;
    MultiLineEdit* _desc;
    QCheckBox* _unique;
    ListView* _columns;
};

#endif // INDEX_EDIT_H
