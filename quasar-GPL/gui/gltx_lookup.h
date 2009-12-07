// $Id: gltx_lookup.h,v 1.8 2004/09/28 16:45:46 bpepers Exp $
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

#ifndef GLTX_LOOKUP_H
#define GLTX_LOOKUP_H

#include "quasar_lookup.h"
#include "tx_combo.h"

class GltxLookup: public QuasarLookup {
    Q_OBJECT
public:
    GltxLookup(MainWindow* main, QWidget* parent, int type=-1);
    GltxLookup(QuasarClient* quasar, QWidget* parent, int type=-1);
    ~GltxLookup();

    QString lookupById(Id gltx_id);
    vector<DataPair> lookupByText(const QString& text);

    bool activeOnly;
    TxCombo* type;

public slots:
    void refresh();
    QWidget* slotNew();
    QWidget* slotEdit(Id id);
};

#endif // GLTX_LOOKUP_H
