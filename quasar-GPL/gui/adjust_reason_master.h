// $Id: adjust_reason_master.h,v 1.2 2005/01/09 10:02:53 bpepers Exp $
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

#ifndef ADJUST_REASON_MASTER_H
#define ADJUST_REASON_MASTER_H

#include "adjust_reason.h"
#include "data_window.h"

class LineEdit;
class DoubleEdit;
class LookupEdit;

class AdjustReasonMaster: public DataWindow {
    Q_OBJECT
public:
    AdjustReasonMaster(MainWindow*, Id reason_id=INVALID_ID);
    ~AdjustReasonMaster();

    void setName(const QString& name);

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

    AdjustReason _orig;
    AdjustReason _curr;

    // Widgets
    LineEdit* _name;
    LineEdit* _number;
    LookupEdit* _account;
};

#endif // ADJUST_REASON_MASTER_H
