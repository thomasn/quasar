// $Id: data_window.h,v 1.25 2004/12/06 18:40:42 bpepers Exp $
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

#ifndef DATA_WINDOW_H
#define DATA_WINDOW_H

#include "edit_window.h"
#include "data_object.h"

class QCheckBox;

class DataWindow: public EditWindow {
    Q_OBJECT
public:
    DataWindow(MainWindow* main, const char* name, Id id);
    ~DataWindow();

public slots:
    virtual bool slotOk();
    virtual bool slotNext();
    virtual void slotCancel();
    virtual void slotDelete();
    virtual void slotClone();
    virtual void slotPrint();

signals:
    void created(Id id);
    void deleted(Id id);
    void updated(Id id);

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void finalize();
    virtual void startEdit();

    virtual void clearErrors();
    virtual void showErrors();

    // Methods children have to implement
    virtual void oldItem() = 0;
    virtual void newItem() = 0;
    virtual void cloneFrom(Id id) = 0;
    virtual bool fileItem() = 0;
    virtual bool deleteItem() = 0;
    virtual void restoreItem() = 0;
    virtual void cloneItem() = 0;
    virtual bool isChanged() = 0;
    virtual void dataToWidget() = 0;
    virtual void widgetToData() = 0;
    virtual bool dataValidate();
    virtual bool saveItem(bool ask=false);
    virtual void printItem(bool ask=false);

    QString cloneName();
    bool checkGltxUsed(const QString& number, int type, Id id);

    Id _id;
    QPushButton* _ok;
    QPushButton* _next;
    QPushButton* _cancel;
    QCheckBox* _inactive;
    QWidget* _firstField;
    bool _busy;
};

#endif // DATA_WINDOW_H
