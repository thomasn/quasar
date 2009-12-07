// $Id: order_template_lookup.h,v 1.5 2004/02/18 22:04:32 bpepers Exp $
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

#ifndef ORDER_TEMPLATE_LOOKUP_H
#define ORDER_TEMPLATE_LOOKUP_H

#include "quasar_lookup.h"

class OrderTemplateLookup: public QuasarLookup {
    Q_OBJECT
public:
    OrderTemplateLookup(MainWindow* main, QWidget* parent);
    OrderTemplateLookup(QuasarClient* quasar, QWidget* parent);
    ~OrderTemplateLookup();

    QString lookupById(Id template_id);
    vector<DataPair> lookupByText(const QString& text);

    bool activeOnly;

public slots:
    void refresh();
    QWidget* slotNew();
    QWidget* slotEdit(Id id);
};

#endif // ORDER_TEMPLATE_LOOKUP_H
