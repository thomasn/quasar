// $Id: model_window.h,v 1.4 2004/11/29 17:35:58 bpepers Exp $
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

#ifndef MODEL_WINDOW_H
#define MODEL_WINDOW_H

#include <qmainwindow.h>
#include "data_model.h"

class LineEdit;
class MultiLineEdit;
class ListView;
class QPushButton;

class ModelWindow: public QMainWindow {
    Q_OBJECT
public:
    ModelWindow();
    ~ModelWindow();

    // Load a model file
    bool load(const QString& filename);

    // Allow const access to current model
    const DataModel& model() const { return _curr; }

    // Make changes to current model (emit signals)
    void add(const BusinessObject& object);
    void update(const BusinessObject& orig, const BusinessObject& object);
    void remove(const BusinessObject& object);

signals:
    void modelAdd(const QString& name);
    void modelUpdate(const QString& name);
    void modelRemove(const QString& name);

protected slots:
    void slotFileMenu();
    void slotNew();
    void slotOpen();
    bool slotSave();
    bool slotSaveAs();
    void slotAddObject();
    void slotEditObject();
    void slotRemoveObject();
    void slotCurrentObjectChanged();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    void setWidgets();
    void getWidgets();
    void reloadObjects();
    bool checkChanged();
    QString currentObject();

    QString _filename;		// File model was loaded from
    DataModel _orig;		// Model before edits
    DataModel _curr;		// Current model

    LineEdit* _version;
    ListView* _list;
    QPushButton* _add;
    QPushButton* _edit;
    QPushButton* _remove;
    QPopupMenu* _file;
};

#endif // MODEL_WINDOW_H
