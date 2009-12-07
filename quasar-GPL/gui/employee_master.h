// $Id: employee_master.h,v 1.20 2005/02/03 08:19:28 bpepers Exp $
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

#ifndef EMPLOYEE_MASTER_H
#define EMPLOYEE_MASTER_H

#include "employee.h"
#include "data_window.h"
#include "variant.h"

class QLabel;
class QCheckBox;
class QComboBox;
class LineEdit;
class Table;

class EmployeeMaster: public DataWindow {
    Q_OBJECT
public:
    EmployeeMaster(MainWindow* main, Id employee_id=INVALID_ID);
    ~EmployeeMaster();

protected slots:
    void companyChanged(bool company);
    void groupCellChanged(int row, int col, Variant old);
    void extraFocusNext(bool& leave, int& newRow, int& newcol, int type);
    void slotCreateData();
    void slotDeleteData();
    void slotRenameData();
    void slotEmail();

protected:
    virtual void oldItem();
    virtual void newItem();
    virtual void cloneFrom(Id id);
    virtual bool fileItem();
    virtual bool deleteItem();
    virtual void restoreItem();
    virtual void cloneItem();
    virtual bool isChanged();
    virtual void dataToWidget();
    virtual void widgetToData();

    Employee _orig;
    Employee _curr;

    QString _companyName;
    QString _lastName;
    QString _firstName;
    QString _contact;

    // Widgets
    QCheckBox* _company;
    QLabel* _label1;
    QLabel* _label2;
    LineEdit* _entry1;
    LineEdit* _entry2;
    LineEdit* _number;
    LineEdit* _street;
    LineEdit* _street2;
    LineEdit* _city;
    LineEdit* _province;
    LineEdit* _country;
    LineEdit* _postal;
    LineEdit* _phone_num;
    LineEdit* _phone2_num;
    LineEdit* _fax_num;
    LineEdit* _email;
    LineEdit* _web_page;
    LineEdit* _posPasswd;
    QComboBox* _posLevel;
    Table* _groups;
    Table* _extra;
};

#endif // EMPLOYEE_MASTER_H
