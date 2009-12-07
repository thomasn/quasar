// $Id: model_edit.h,v 1.2 2004/11/30 03:52:56 bpepers Exp $
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

#ifndef MODEL_EDIT_H
#define MODEL_EDIT_H

#include <qmainwindow.h>
#include <qguardedptr.h>
#include "data_model.h"

class ModelMainWindow;
class ObjectEdit;
class LineEdit;
class MultiLineEdit;
class ListView;

typedef QGuardedPtr<ObjectEdit> ObjectEditPtr;

class ModelEdit: public QMainWindow {
    Q_OBJECT
public:
    ModelEdit(ModelMainWindow* main, const QString& version);
    virtual ~ModelEdit();

    void insert(const BusinessObject& object);
    void update(const BusinessObject& orig, const BusinessObject& object);
    void remove(const BusinessObject& object);

protected slots:
    bool slotOk();
    void slotReset();
    void slotCancel();
    void slotAddObject();
    void slotEditObject();
    void slotRemoveObject();
    void slotAddUpdate();
    void slotEditUpdate();
    void slotRemoveUpdate();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    void setWidgets();
    void getWidgets();
    void refreshObjects();
    void refreshUpdates();
    bool checkChanged();
    BusinessObject* currentObject();
    UpdateDefn* currentUpdate();

    ModelMainWindow* _main;			// Main window
    QValueList<ObjectEditPtr> _objectEdits;	// Object editors

    DataModel _orig;		// Original data model
    DataModel _curr;		// Current edited data model

    LineEdit* _version;
    MultiLineEdit* _desc;
    LineEdit* _from;
    ListView* _objects;
    ListView* _updates;
};

#endif // MODEL_EDIT_H
