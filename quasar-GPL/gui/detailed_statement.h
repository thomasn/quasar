// $Id: detailed_statement.h,v 1.4 2004/01/31 01:50:31 arandell Exp $
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

#ifndef DETAILED_STATEMENT_H
#define DETAILED_STATEMENT_H

#include "quasar_window.h"
#include "gltx.h"

class LookupEdit;
class DatePopup;
class ListView;
class MoneyEdit;

class DetailedStatement: public QuasarWindow {
    Q_OBJECT
public:
    DetailedStatement(MainWindow* main);
    ~DetailedStatement();

    void setStoreId(Id store_id);
    void setCustomerId(Id customer_id);

protected slots:
    void slotPickLine();
    void slotRefresh();
    void slotPrint();

protected:
    LookupEdit* _customer;
    LookupEdit* _store;
    DatePopup* _from;
    DatePopup* _to;
    ListView* _lines;
    ListView* _taxes;
    MoneyEdit* _chargeTotal;
    MoneyEdit* _creditTotal;
    MoneyEdit* _dueTotal;
};

#endif // DETAILED_STATEMENT_H
