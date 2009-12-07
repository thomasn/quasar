// $Id: invoice_info.h,v 1.5 2004/01/31 01:50:31 arandell Exp $
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

#ifndef INVOICE_INFO_H
#define INVOICE_INFO_H

#include <qdialog.h>
#include "gltx.h"
#include "term.h"

class MainWindow;
class MoneyEdit;
class LineEdit;
class DateEdit;
class QPushButton;

class InvoiceInfo: public QDialog {
    Q_OBJECT
public:
    InvoiceInfo(MainWindow* main, QWidget* parent);
    virtual ~InvoiceInfo();

    void setInvoice(const Gltx& gltx, const Term& term, fixed taken);

signals:
    void newDiscount(fixed new_disc);

protected slots:
    void slotViewInvoice();
    void slotTakeDiscount();
    void slotClearDiscount();
    void slotDiscountChanged();

protected:
    MainWindow* _main;
    Gltx _gltx;
    Term _term;

    LineEdit* _number;
    MoneyEdit* _total;
    MoneyEdit* _paid;
    MoneyEdit* _due;
    LineEdit* _terms;
    DateEdit* _date;
    DateEdit* _dueDate;
    DateEdit* _discDate;
    MoneyEdit* _discount;
    MoneyEdit* _taken;
    QPushButton* _view;
};

#endif // INVOICE_INFO_H
