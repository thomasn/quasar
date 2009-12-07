// $Id: service_charges.h,v 1.5 2005/01/30 04:25:31 bpepers Exp $
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

#ifndef SERVICE_CHARGES_H
#define SERVICE_CHARGES_H

#include "quasar_window.h"
#include "gltx.h"
#include "term.h"

class DatePopup;
class LookupEdit;
class ListView;

class ServiceCharges: public QuasarWindow {
    Q_OBJECT
public:
    ServiceCharges(MainWindow* main);
    ~ServiceCharges();

protected slots:
    void slotCalculate();
    void slotPost();

protected:
    DatePopup* _start;
    DatePopup* _end;
    LookupEdit* _store;
    LookupEdit* _account;
    ListView* _charges;

    fixed balanceOn(const Gltx& gltx, QDate date);

    vector<Gltx> _gltxs;
    bool findGltx(Id gltx_id, Gltx& gltx);

    vector<Term> _terms;
    bool findTerm(Id term_id, Term& term);

    vector<Id> _invoice_ids;
    vector<Id> _invoice_term_ids;
    Id invoiceTermId(Id invoice_id);
};

#endif // SERVICE_CHARGES_H
