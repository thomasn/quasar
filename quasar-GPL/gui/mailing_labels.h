// $Id: mailing_labels.h,v 1.7 2005/01/31 23:28:32 bpepers Exp $
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

#ifndef MAILING_LABELS_H
#define MAILING_LABELS_H

#include "quasar_window.h"
#include "date_time.h"
#include "fixed.h"

class ListView;
class NumberEdit;
class IntegerEdit;
class QComboBox;
class QTabWidget;

class MailingLabels: public QuasarWindow {
    Q_OBJECT
public:
    MailingLabels(MainWindow* main);
    ~MailingLabels();

protected slots:
    void slotRefresh();
    void slotSelectAll();
    void slotSelectNone();
    void slotPrint();

protected:
    QTabWidget* _tabs;
    ListView* _customer;
    ListView* _vendor;
    ListView* _employee;
    ListView* _personal;
    QComboBox* _type;
    IntegerEdit* _number;
    IntegerEdit* _row;
    IntegerEdit* _col;
};

#endif // MAILING_LABELS_H
