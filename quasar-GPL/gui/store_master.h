// $Id: store_master.h,v 1.9 2005/03/15 15:34:35 bpepers Exp $
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

#ifndef STORE_MASTER_H
#define STORE_MASTER_H

#include "store.h"
#include "data_window.h"

class LineEdit;
class QGroupBox;
class QCheckBox;
class LookupEdit;

class StoreMaster: public DataWindow {
    Q_OBJECT
public:
    StoreMaster(MainWindow*, Id store_id=INVALID_ID);
    ~StoreMaster();

protected slots:
    void slotHasAddrChanged();

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

    Store _orig;
    Store _curr;

    // Widgets
    LineEdit* _name;
    LineEdit* _number;
    LineEdit* _contact;
    LookupEdit* _company;
    QCheckBox* _has_addr;
    QGroupBox* _addr;
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
    QCheckBox* _can_sell;
    LookupEdit* _transfer;
};

#endif // STORE_MASTER_H
