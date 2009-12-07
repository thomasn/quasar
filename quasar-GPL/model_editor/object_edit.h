// $Id: object_edit.h,v 1.6 2004/11/30 03:52:56 bpepers Exp $
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

#ifndef OBJECT_EDIT_H
#define OBJECT_EDIT_H

#include <qmainwindow.h>
#include <qguardedptr.h>
#include "data_model.h"

class ModelEdit;
class TableEdit;
class LineEdit;
class IntegerEdit;
class MultiLineEdit;
class ListView;

typedef QGuardedPtr<TableEdit> TableEditPtr;

class ObjectEdit: public QMainWindow {
    Q_OBJECT
public:
    ObjectEdit(ModelEdit* parent, const BusinessObject& object);
    virtual ~ObjectEdit();

    void insert(const TableDefn& table);
    void update(const TableDefn& orig, const TableDefn& table);
    void remove(const TableDefn& table);

protected slots:
    bool slotOk();
    void slotNext();
    void slotReset();
    void slotCancel();
    void slotAddTable();
    void slotEditTable();
    void slotRemoveTable();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    void setWidgets();
    void getWidgets();
    void refreshTables();
    bool checkChanged();
    TableDefn* currentTable();

    ModelEdit* _edit;				// Model edit window
    QValueList<TableEditPtr> _tableEdits;	// Table editors

    BusinessObject _orig;	// Original business object
    BusinessObject _curr;	// Current edited business object

    LineEdit* _name;
    IntegerEdit* _objectNum;
    MultiLineEdit* _desc;
    ListView* _list;
};

#endif // OBJECT_EDIT_H
