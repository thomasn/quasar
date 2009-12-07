// $Id: charge_master.h,v 1.6 2004/11/12 09:28:03 bpepers Exp $
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

#ifndef CHARGE_MASTER_H
#define CHARGE_MASTER_H

#include "charge.h"
#include "data_window.h"

class LineEdit;
class LookupEdit;
class NumberEdit;
class QRadioButton;
class QComboBox;
class QCheckBox;

class ChargeMaster: public DataWindow {
    Q_OBJECT
public:
    ChargeMaster(MainWindow* main, Id charge_id=INVALID_ID);
    ~ChargeMaster();

protected slots:
    void slotCalcChanged();

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

    Charge _orig;
    Charge _curr;

    // Widgets
    LineEdit* _name;
    LookupEdit* _account;
    LookupEdit* _tax;
    QCheckBox* _incTax;
    QRadioButton* _calcManual;
    QRadioButton* _calcCost;
    NumberEdit* _percent;
    QRadioButton* _calcWeight;
    NumberEdit* _dollar;
    QComboBox* _alloc_method;
};

#endif // CHARGE_MASTER_H
