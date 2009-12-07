// $Id: model_main_window.h,v 1.1 2004/11/29 17:38:00 bpepers Exp $
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

#ifndef MODEL_MAIN_WINDOW_H
#define MODEL_MAIN_WINDOW_H

#include <qmainwindow.h>
#include "data_model.h"

class LineEdit;
class MultiLineEdit;
class ListView;
class QPushButton;

class ModelMainWindow: public QMainWindow {
    Q_OBJECT
public:
    ModelMainWindow();
    ~ModelMainWindow();

protected slots:
    void slotNewModel();
    void slotEditModel();
    void slotRemoveModel();
    void slotCurrentModelChanged();
    void slotRefresh();

protected:
    void closeEvent(QCloseEvent* e);
    void createWidgets();
    QString currentModel();

    QValueList<DataModel> _models;
    QString _latestVersion;

    ListView* _list;
    QPushButton* _new;
    QPushButton* _edit;
    QPushButton* _remove;
};

#endif // MODEL_MAIN_WINDOW_H
