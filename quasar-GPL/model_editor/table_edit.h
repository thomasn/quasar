// $Id: table_edit.h,v 1.4 2004/12/19 10:40:10 bpepers Exp $
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

#ifndef TABLE_EDIT_H
#define TABLE_EDIT_H

#include <qmainwindow.h>
#include <qguardedptr.h>
#include "data_model.h"

class ObjectEdit;
class ColumnEdit;
class ConstraintEdit;
class IndexEdit;
class LineEdit;
class MultiLineEdit;
class ListView;

typedef QGuardedPtr<ColumnEdit> ColumnEditPtr;
typedef QGuardedPtr<ConstraintEdit> ConstraintEditPtr;
typedef QGuardedPtr<IndexEdit> IndexEditPtr;

class TableEdit: public QMainWindow {
    Q_OBJECT
public:
    TableEdit(ObjectEdit* edit, const TableDefn& table);
    virtual ~TableEdit();

    const TableDefn& table() const { return _curr; }

    void insert(const ColumnDefn& column);
    void update(const ColumnDefn& orig, const ColumnDefn& column);
    void remove(const ColumnDefn& column);

    void insert(const ConstraintDefn& constraint);
    void update(const ConstraintDefn& orig, const ConstraintDefn& constraint);
    void remove(const ConstraintDefn& constraint);

    void insert(const IndexDefn& index);
    void update(const IndexDefn& orig, const IndexDefn& index);
    void remove(const IndexDefn& index);

protected slots:
    bool slotOk();
    void slotNext();
    void slotReset();
    void slotCancel();
    void slotAddColumn();
    void slotEditColumn();
    void slotRemoveColumn();
    void slotAddConstraint();
    void slotEditConstraint();
    void slotRemoveConstraint();
    void slotAddIndex();
    void slotEditIndex();
    void slotRemoveIndex();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    void setWidgets();
    void getWidgets();
    void refreshColumns();
    void refreshConstraints();
    void refreshIndexes();
    bool checkChanged();
    ColumnDefn* currentColumn();
    ConstraintDefn* currentConstraint();
    IndexDefn* currentIndex();

    ObjectEdit* _edit;				// Object edit window
    QValueList<ColumnEditPtr> _columnEdits;	// Column editors
    QValueList<ConstraintEditPtr> _constEdits;	// Constraint editors
    QValueList<IndexEditPtr> _indexEdits;	// Index editors

    TableDefn _orig;		// Original table
    TableDefn _curr;		// Current edited table

    LineEdit* _name;
    MultiLineEdit* _desc;
    ListView* _columns;
    ListView* _constraints;
    ListView* _indexes;
};

#endif // TABLE_EDIT_H
